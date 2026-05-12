#include "qt-ai-desktop/llamacpp_backend.h"
#include <QThread>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDebug>

// 条件编译：如果 llama.cpp 可用则编译实现
#ifdef QTAI_HAS_LLAMA_CPP

#include "llama.h"
#include "common.h"

namespace QtAIDesktop {

// 注册 llama.cpp 后端
QTAI_REGISTER_BACKEND(LlamaCppBackend, "llamacpp")

// ============================================================================
// 实现
// ============================================================================

LlamaCppBackend::LlamaCppBackend() = default;

LlamaCppBackend::~LlamaCppBackend() {
    shutdown();
}

QString LlamaCppBackend::version() const {
    return QString::fromLatin1(LLAMA_CPP_VERSION);
}

bool LlamaCppBackend::initialize(const QVariantMap &options) {
    QMutexLocker locker(&m_mutex);
    
    if (m_initialized) {
        return true;
    }
    
    // 解析配置
    m_config.contextLength = options.value("contextLength", 2048).toInt();
    m_config.batchSize = options.value("batchSize", 512).toInt();
    m_config.gpuLayers = options.value("gpuLayers", 99).toInt();
    m_config.useMmap = options.value("useMmap", true).toBool();
    m_config.useMlock = options.value("useMlock", false).toBool();
    m_config.threads = options.value("threads", QThread::idealThreadCount()).toInt();
    m_config.flashAttention = options.value("flashAttention", false).toBool();
    
    // 初始化 llama.cpp 后端
    ggml_backend_load_all();
    
    m_initialized = true;
    
    qDebug() << "[LlamaCpp] Initialized:"
             << "ctx=" << m_config.contextLength
             << "gpu=" << m_config.gpuLayers
             << "threads=" << m_config.threads;
    
    return true;
}

void LlamaCppBackend::shutdown() {
    QMutexLocker locker(&m_mutex);
    
    unloadModel();
    m_initialized = false;
    
    qDebug() << "[LlamaCpp] Shutdown";
}

LoadResult LlamaCppBackend::loadModel(const QString &modelPath, 
                                       const QVariantMap &options) {
    QMutexLocker locker(&m_mutex);
    
    LoadResult result;
    
    if (!m_initialized) {
        result.error = "Backend not initialized";
        return result;
    }
    
    // 检查文件
    QFileInfo fileInfo(modelPath);
    if (!fileInfo.exists()) {
        result.error = QString("Model file not found: %1").arg(modelPath);
        qWarning() << "[LlamaCpp]" << result.error;
        return result;
    }
    
    // 卸载旧模型
    if (m_modelLoaded) {
        unloadModel();
    }
    
    // 更新配置
    m_config.contextLength = options.value("contextLength", m_config.contextLength).toInt();
    m_config.gpuLayers = options.value("gpuLayers", m_config.gpuLayers).toInt();
    
    // 设置模型参数
    llama_model_params model_params = llama_model_default_params();
    model_params.n_gpu_layers = m_config.gpuLayers;
    model_params.use_mmap = m_config.useMmap;
    model_params.use_mlock = m_config.useMlock;
    
    // 加载模型
    std::string path_str = modelPath.toStdString();
    m_model = llama_model_load_from_file(path_str.c_str(), model_params);
    
    if (!m_model) {
        result.error = "Failed to load model";
        qWarning() << "[LlamaCpp]" << result.error;
        return result;
    }
    
    // 创建上下文
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = m_config.contextLength;
    ctx_params.n_batch = m_config.batchSize;
    ctx_params.n_threads = m_config.threads;
    ctx_params.n_threads_batch = m_config.threads;
    ctx_params.flash_attn = m_config.flashAttention;
    
    m_ctx = llama_new_context_with_model(m_model, ctx_params);
    
    if (!m_ctx) {
        llama_model_free(m_model);
        m_model = nullptr;
        result.error = "Failed to create context";
        qWarning() << "[LlamaCpp]" << result.error;
        return result;
    }
    
    // 初始化采样器
    llama_sampler_chain_params sparams = llama_sampler_chain_default_params();
    m_sampler = llama_sampler_chain_init(sparams);
    
    llama_sampler_chain_add(m_sampler, llama_sampler_init_temp(m_genParams.temperature));
    llama_sampler_chain_add(m_sampler, llama_sampler_init_top_p(m_genParams.topP, 1));
    llama_sampler_chain_add(m_sampler, llama_sampler_init_dist(m_genParams.seed));
    
    // 设置状态
    m_modelLoaded = true;
    m_modelName = fileInfo.completeBaseName();
    m_modelPath = modelPath;
    m_contextSize = m_config.contextLength;
    
    // 获取模型信息
    result.success = true;
    result.modelSize = fileInfo.size();
    result.memoryRequired = llama_model_size(m_model);
    result.contextLength = m_config.contextLength;
    
    qDebug() << "[LlamaCpp] Model loaded:" << m_modelName
             << "size=" << (result.modelSize / 1024 / 1024) << "MB"
             << "ctx=" << m_config.contextLength;
    
    return result;
}

void LlamaCppBackend::unloadModel() {
    if (!m_modelLoaded) {
        return;
    }
    
    if (m_sampler) {
        llama_sampler_free(m_sampler);
        m_sampler = nullptr;
    }
    
    if (m_ctx) {
        llama_free(m_ctx);
        m_ctx = nullptr;
    }
    
    if (m_model) {
        llama_model_free(m_model);
        m_model = nullptr;
    }
    
    m_modelLoaded = false;
    m_modelName.clear();
    m_modelPath.clear();
    
    qDebug() << "[LlamaCpp] Model unloaded";
}

bool LlamaCppBackend::isModelLoaded() const {
    return m_modelLoaded;
}

QString LlamaCppBackend::loadedModelName() const {
    return m_modelName;
}

std::vector<int> LlamaCppBackend::tokenize(const QString &text, bool addBos) {
    std::string str = text.toStdString();
    std::vector<int> tokens(llama_n_vocab(m_model) + str.length());
    
    int n = llama_tokenize(
        m_model, 
        str.c_str(), 
        str.length(), 
        tokens.data(), 
        tokens.size(), 
        addBos, 
        false
    );
    
    tokens.resize(n);
    return tokens;
}

QString LlamaCppBackend::detokenize(const std::vector<int> &tokens) {
    std::string result;
    for (int token : tokens) {
        char buf[256];
        int n = llama_token_to_piece(m_model, token, buf, sizeof(buf), 0, false);
        if (n > 0) {
            result.append(buf, n);
        }
    }
    return QString::fromStdString(result);
}

bool LlamaCppBackend::evaluateTokens(const std::vector<int> &tokens) {
    int n = llama_decode(m_ctx, llama_batch_get_one(tokens.data(), tokens.size()));
    return n == 0;
}

int LlamaCppBackend::sampleToken() {
    return llama_sampler_sample(m_sampler, m_ctx, -1);
}

bool LlamaCppBackend::isStopToken(int token) const {
    return llama_token_is_eog(m_model, token);
}

InferenceResult LlamaCppBackend::infer(const QString &prompt, 
                                        const QVariantMap &params) {
    QMutexLocker locker(&m_mutex);
    
    InferenceResult result;
    
    if (!m_modelLoaded) {
        result.error = "Model not loaded";
        return result;
    }
    
    // 解析参数
    int maxTokens = params.value("maxTokens", m_genParams.maxTokens).toInt();
    float temp = params.value("temperature", m_genParams.temperature).toFloat();
    
    // 更新采样器温度
    if (temp != m_genParams.temperature) {
        m_genParams.temperature = temp;
        // 重建采样器
        if (m_sampler) {
            llama_sampler_free(m_sampler);
        }
        llama_sampler_chain_params sparams = llama_sampler_chain_default_params();
        m_sampler = llama_sampler_chain_init(sparams);
        llama_sampler_chain_add(m_sampler, llama_sampler_init_temp(m_genParams.temperature));
        llama_sampler_chain_add(m_sampler, llama_sampler_init_top_p(m_genParams.topP, 1));
        llama_sampler_chain_add(m_sampler, llama_sampler_init_dist(m_genParams.seed));
    }
    
    m_stopRequested = false;
    
    QElapsedTimer timer;
    timer.start();
    
    // 构建完整提示（含系统提示）
    QString fullPrompt;
    if (!m_systemPrompt.isEmpty()) {
        fullPrompt = m_systemPrompt + "\n\n" + prompt;
    } else {
        fullPrompt = prompt;
    }
    
    // Tokenize
    auto tokens = tokenize(fullPrompt, true);
    
    // 评估提示 tokens
    if (!evaluateTokens(tokens)) {
        result.error = "Failed to evaluate prompt";
        return result;
    }
    
    // 生成循环
    QString generatedText;
    int n_generated = 0;
    
    while (n_generated < maxTokens && !m_stopRequested) {
        // 采样下一个 token
        int new_token = sampleToken();
        
        // 检查是否结束
        if (isStopToken(new_token)) {
            break;
        }
        
        // 解码 token
        QString token_text = detokenize({new_token});
        generatedText += token_text;
        n_generated++;
        
        // 流式回调
        if (m_streamCallback) {
            m_streamCallback(token_text);
        }
        
        // 评估新 token
        if (!evaluateTokens({new_token})) {
            result.error = "Failed to evaluate token";
            break;
        }
        
        // 处理事件
        if (n_generated % 10 == 0) {
            QCoreApplication::processEvents();
        }
    }
    
    qint64 elapsed = timer.elapsed();
    
    // 更新统计
    m_totalInferences++;
    m_totalTokens += n_generated;
    m_totalTimeMs += elapsed;
    
    result.success = true;
    result.text = generatedText;
    result.tokensGenerated = n_generated;
    result.tokensPerSecond = n_generated / (elapsed / 1000.0);
    result.durationMs = elapsed;
    
    qDebug() << "[LlamaCpp] Inference:" << n_generated << "tokens in" 
             << elapsed << "ms" << "(" << result.tokensPerSecond << "tok/s)";
    
    return result;
}

void LlamaCppBackend::setStreamCallback(std::function<void(const QString &)> callback) {
    m_streamCallback = std::move(callback);
}

void LlamaCppBackend::stopGeneration() {
    m_stopRequested = true;
}

QVariantMap LlamaCppBackend::statistics() const {
    QMutexLocker locker(&m_mutex);
    
    QVariantMap stats;
    stats["backend"] = name();
    stats["version"] = version();
    stats["totalInferences"] = m_totalInferences;
    stats["totalTokens"] = m_totalTokens;
    stats["totalTimeMs"] = m_totalTimeMs;
    stats["avgTokensPerSecond"] = m_totalTimeMs > 0 
        ? m_totalTokens / (m_totalTimeMs / 1000.0) 
        : 0.0;
    stats["modelLoaded"] = m_modelLoaded;
    stats["modelName"] = m_modelName;
    stats["contextSize"] = m_contextSize;
    stats["gpuLayers"] = m_config.gpuLayers;
    
    return stats;
}

void LlamaCppBackend::setSystemPrompt(const QString &prompt) {
    QMutexLocker locker(&m_mutex);
    m_systemPrompt = prompt;
    qDebug() << "[LlamaCpp] System prompt set:" << prompt.left(50) << "...";
}

QVariantMap LlamaCppBackend::modelDetails() const {
    QMutexLocker locker(&m_mutex);
    
    QVariantMap details;
    
    if (!m_modelLoaded || !m_model) {
        return details;
    }
    
    details["name"] = m_modelName;
    details["path"] = m_modelPath;
    details["contextLength"] = m_config.contextLength;
    details["gpuLayers"] = m_config.gpuLayers;
    details["vocabSize"] = llama_n_vocab(m_model);
    details["embeddingSize"] = llama_model_n_embd(m_model);
    details["layers"] = llama_model_n_layer(m_model);
    details["trainingContextLength"] = llama_model_n_ctx_train(m_model);
    
    return details;
}

QVector<float> LlamaCppBackend::embeddings(const QString &text) {
    QMutexLocker locker(&m_mutex);
    
    if (!m_modelLoaded) {
        qWarning() << "[LlamaCpp] Model not loaded";
        return {};
    }
    
    // Tokenize
    auto tokens = tokenize(text, true);
    
    // 评估
    if (!evaluateTokens(tokens)) {
        qWarning() << "[LlamaCpp] Failed to evaluate for embeddings";
        return {};
    }
    
    // 获取嵌入
    int n_embd = llama_model_n_embd(m_model);
    const float *embd = llama_get_embeddings(m_ctx);
    
    if (!embd) {
        return {};
    }
    
    return QVector<float>(embd, embd + n_embd);
}

} // namespace QtAIDesktop

#else // !QTAI_HAS_LLAMA_CPP

// ============================================================================
// 无 llama.cpp 时的存根实现
// ============================================================================

namespace QtAIDesktop {

LlamaCppBackend::LlamaCppBackend() = default;
LlamaCppBackend::~LlamaCppBackend() = default;

QString LlamaCppBackend::version() const { return "N/A (not compiled)"; }

bool LlamaCppBackend::initialize(const QVariantMap &) {
    qWarning() << "[LlamaCpp] Not compiled with llama.cpp support";
    return false;
}

void LlamaCppBackend::shutdown() {}
LoadResult LlamaCppBackend::loadModel(const QString &, const QVariantMap &) {
    LoadResult r;
    r.error = "llama.cpp not available";
    return r;
}
void LlamaCppBackend::unloadModel() {}
bool LlamaCppBackend::isModelLoaded() const { return false; }
QString LlamaCppBackend::loadedModelName() const { return {}; }
InferenceResult LlamaCppBackend::infer(const QString &, const QVariantMap &) {
    InferenceResult r;
    r.error = "llama.cpp not available";
    return r;
}
void LlamaCppBackend::setStreamCallback(std::function<void(const QString &)>) {}
void LlamaCppBackend::stopGeneration() {}
QVariantMap LlamaCppBackend::statistics() const { return {}; }
void LlamaCppBackend::setSystemPrompt(const QString &) {}
QVariantMap LlamaCppBackend::modelDetails() const { return {}; }
QVector<float> LlamaCppBackend::embeddings(const QString &) { return {}; }

// 不注册后端（因为不可用）

} // namespace QtAIDesktop

#endif // QTAI_HAS_LLAMA_CPP
