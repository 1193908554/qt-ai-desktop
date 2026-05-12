#include "qt-ai-desktop/onnxruntime_backend.h"
#include "qt-ai-desktop/logger.h"

#ifdef QTAI_HAS_ONNXRUNTIME
#include <onnxruntime_cxx_api.h>
#endif

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRandomGenerator>
#include <QtMath>
#include <numeric>
#include <algorithm>

namespace qt_ai_desktop {

ONNXRuntimeBackend::ONNXRuntimeBackend()
    : IBackend(nullptr)
{
}

ONNXRuntimeBackend::~ONNXRuntimeBackend() {
    m_isGenerating = false;
}

bool ONNXRuntimeBackend::initialize() {
    QMutexLocker locker(&m_mutex);
    
    if (m_initialized) return true;
    
#ifdef QTAI_HAS_ONNXRUNTIME
    try {
        // 创建 ONNX Runtime 环境
        m_env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "qt-ai-desktop");
        m_memoryInfo = std::make_unique<Ort::MemoryInfo>(
            Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault));
        
        m_initialized = true;
        Logger::info("ONNX Runtime backend initialized");
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to initialize ONNX Runtime: {}", e.what());
        return false;
    }
#else
    Logger::warn("ONNX Runtime support not compiled");
    return false;
#endif
}

bool ONNXRuntimeBackend::loadModel(const std::string& modelPath) {
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized) {
        Logger::error("Backend not initialized");
        return false;
    }
    
#ifdef QTAI_HAS_ONNXRUNTIME
    try {
        // 配置会话选项
        Ort::SessionOptions sessionOptions;
        sessionOptions.SetIntraOpNumThreads(m_numThreads);
        sessionOptions.SetGraphOptimizationLevel(
            static_cast<GraphOptimizationLevel>(m_graphOptimizationLevel));
        
        // 设置执行提供者
        if (m_executionProvider == "cuda") {
            // TODO: 配置 CUDA
        } else if (m_executionProvider == "directml") {
            // TODO: 配置 DirectML
        }
        
        // 创建会话
        m_session = std::make_unique<Ort::Session>(
            *m_env, modelPath.c_str(), sessionOptions);
        
        m_modelPath = modelPath;
        m_modelLoaded = true;
        
        Logger::info("Model loaded: {}", modelPath);
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to load model: {}", e.what());
        return false;
    }
#else
    return false;
#endif
}

bool ONNXRuntimeBackend::isModelLoaded() const {
    return m_modelLoaded;
}

std::string ONNXRuntimeBackend::generate(const std::string& prompt,
                                          const GenerationParams& params) {
    if (!m_modelLoaded) return "";
    
    // 使用流式接口收集完整结果
    std::string result;
    generateStream(prompt, [&result](const std::string& token) {
        result += token;
    }, params);
    return result;
}

void ONNXRuntimeBackend::generateStream(const std::string& prompt,
                                         TokenCallback callback,
                                         const GenerationParams& params) {
    if (!m_modelLoaded || !callback) return;
    
    m_isGenerating = true;
    
    // Tokenize 输入
    auto inputIds = tokenize(prompt);
    
    int tokensGenerated = 0;
    std::string currentText;
    
    while (m_isGenerating && tokensGenerated < params.maxTokens) {
        // 运行推理
        std::vector<float> logits;
        if (!runInference(inputIds, logits)) {
            Logger::error("Inference failed");
            break;
        }
        
        // 采样下一个 token
        int64_t nextToken = sampleToken(logits, params);
        
        // 检查 EOS
        if (nextToken == m_eosTokenId) break;
        
        // 解码 token
        std::string tokenStr = detokenize({nextToken});
        
        // 回调
        callback(tokenStr);
        currentText += tokenStr;
        
        // 添加到输入序列
        inputIds.push_back(nextToken);
        tokensGenerated++;
    }
    
    m_isGenerating = false;
}

void ONNXRuntimeBackend::stopGeneration() {
    m_isGenerating = false;
}

bool ONNXRuntimeBackend::isGenerating() const {
    return m_isGenerating;
}

std::string ONNXRuntimeBackend::backendName() const {
    return "onnxruntime";
}

std::string ONNXRuntimeBackend::modelName() const {
    return m_modelPath;
}

BackendCapabilities ONNXRuntimeBackend::capabilities() const {
    return {
        true,   // supportsStreaming
        true,   // supportsStopGeneration
        true,   // supportsModelHotSwap
        true,   // supportsGPU
        false   // supportsEmbeddings
    };
}

bool ONNXRuntimeBackend::setExecutionProvider(const std::string& provider) {
    m_executionProvider = provider;
    Logger::info("Execution provider set to: {}", provider);
    return true;
}

QStringList ONNXRuntimeBackend::availableProviders() const {
    QStringList providers;
    providers << "cpu";
    
#ifdef QTAI_HAS_ONNXRUNTIME
    // 检查可用的执行提供者
    auto available = Ort::GetAvailableProviders();
    for (const auto& provider : available) {
        providers << QString::fromStdString(provider);
    }
#endif
    
    return providers;
}

bool ONNXRuntimeBackend::setNumThreads(int numThreads) {
    m_numThreads = numThreads;
    return true;
}

bool ONNXRuntimeBackend::setGraphOptimizationLevel(int level) {
    m_graphOptimizationLevel = level;
    return true;
}

bool ONNXRuntimeBackend::runInference(const std::vector<int64_t>& inputIds,
                                       std::vector<float>& outputLogits) {
#ifdef QTAI_HAS_ONNXRUNTIME
    try {
        // 准备输入
        std::vector<int64_t> inputShape = {1, static_cast<int64_t>(inputIds.size())};
        Ort::Value inputTensor = Ort::Value::CreateTensor<int64_t>(
            *m_memoryInfo,
            const_cast<int64_t*>(inputIds.data()),
            inputIds.size(),
            inputShape.data(),
            inputShape.size()
        );
        
        // 获取输入输出名称
        auto inputName = m_session->GetInputNameAllocated(0, Ort::AllocatorWithDefaultExample());
        auto outputName = m_session->GetOutputNameAllocated(0, Ort::AllocatorWithDefaultExample());
        
        const char* inputNames[] = {inputName.get()};
        const char* outputNames[] = {outputName.get()};
        
        // 运行推理
        auto outputTensors = m_session->Run(
            Ort::RunOptions{nullptr},
            inputNames,
            &inputTensor,
            1,
            outputNames,
            1
        );
        
        // 获取输出
        auto& outputTensor = outputTensors[0];
        auto outputShape = outputTensor.GetTensorTypeAndShapeInfo().GetShape();
        
        const float* outputData = outputTensor.GetTensorData<float>();
        size_t outputSize = 1;
        for (auto dim : outputShape) {
            outputSize *= dim;
        }
        
        outputLogits.assign(outputData, outputData + outputSize);
        
        return true;
    } catch (const std::exception& e) {
        Logger::error("Inference error: {}", e.what());
        return false;
    }
#else
    return false;
#endif
}

std::vector<int64_t> ONNXRuntimeBackend::tokenize(const std::string& text) {
    // 简化的 tokenize 实现
    std::vector<int64_t> tokens;
    tokens.push_back(m_bosTokenId);
    
    // 简单按字符分割（实际应使用 BPE/SentencePiece）
    for (char c : text) {
        tokens.push_back(static_cast<int64_t>(c));
    }
    
    return tokens;
}

std::string ONNXRuntimeBackend::detokenize(const std::vector<int64_t>& tokens) {
    // 简化的 detokenize 实现
    std::string result;
    for (int64_t token : tokens) {
        if (token > 0 && token < 128) {
            result += static_cast<char>(token);
        }
    }
    return result;
}

int64_t ONNXRuntimeBackend::sampleToken(const std::vector<float>& logits,
                                         const GenerationParams& params) {
    if (logits.empty()) return 0;
    
    // Temperature
    std::vector<float> scaledLogits(logits.size());
    for (size_t i = 0; i < logits.size(); ++i) {
        scaledLogits[i] = logits[i] / params.temperature;
    }
    
    // Softmax
    float maxLogit = *std::max_element(scaledLogits.begin(), scaledLogits.end());
    std::vector<float> probs(logits.size());
    float sum = 0.0f;
    for (size_t i = 0; i < scaledLogits.size(); ++i) {
        probs[i] = std::exp(scaledLogits[i] - maxLogit);
        sum += probs[i];
    }
    for (auto& p : probs) p /= sum;
    
    // Top-K 采样
    std::vector<size_t> indices(probs.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::partial_sort(indices.begin(), 
                      indices.begin() + params.topK,
                      indices.end(),
                      [&probs](size_t a, size_t b) {
                          return probs[a] > probs[b];
                      });
    
    // Top-P 采样
    float cumProb = 0.0f;
    std::vector<std::pair<size_t, float>> candidates;
    for (size_t i = 0; i < params.topK && i < indices.size(); ++i) {
        cumProb += probs[indices[i]];
        candidates.emplace_back(indices[i], probs[indices[i]]);
        if (cumProb >= params.topP) break;
    }
    
    // 随机选择
    float r = QRandomGenerator::global()->generateDouble();
    float acc = 0.0f;
    for (const auto& [idx, prob] : candidates) {
        acc += prob / cumProb;
        if (r <= acc) return static_cast<int64_t>(idx);
    }
    
    return static_cast<int64_t>(candidates[0].first);
}

} // namespace qt_ai_desktop
