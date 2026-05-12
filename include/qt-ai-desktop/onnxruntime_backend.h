#pragma once

#include "qt-ai-desktop/backend.h"
#include <QMutex>
#include <QElapsedTimer>
#include <QString>
#include <QStringList>
#include <QVector>
#include <memory>
#include <string>
#include <functional>
#include <atomic>

// ONNX Runtime 前向声明
namespace Ort {
class Session;
class Env;
class MemoryInfo;
}

namespace qt_ai_desktop {

/**
 * @brief ONNX Runtime 推理后端
 * 
 * 支持 ONNX 模型推理，跨平台 GPU 加速
 * 支持 CUDA、DirectML、CoreML 等加速器
 */
class ONNXRuntimeBackend : public IBackend {
    Q_OBJECT

public:
    ONNXRuntimeBackend();
    ~ONNXRuntimeBackend() override;

    // IBackend 接口实现
    bool initialize() override;
    bool loadModel(const std::string& modelPath) override;
    bool isModelLoaded() const override;
    
    std::string generate(const std::string& prompt, 
                        const GenerationParams& params = {}) override;
    
    void generateStream(const std::string& prompt,
                       TokenCallback callback,
                       const GenerationParams& params = {}) override;
    
    void stopGeneration() override;
    bool isGenerating() const override;
    
    std::string backendName() const override;
    std::string modelName() const override;
    BackendCapabilities capabilities() const override;

    // ONNX 特有方法
    bool setExecutionProvider(const std::string& provider);
    QStringList availableProviders() const;
    bool setNumThreads(int numThreads);
    bool setGraphOptimizationLevel(int level);

private:
    // 模型推理
    bool runInference(const std::vector<int64_t>& inputIds,
                     std::vector<float>& outputLogits);
    
    // Tokenizer (简化实现)
    std::vector<int64_t> tokenize(const std::string& text);
    std::string detokenize(const std::vector<int64_t>& tokens);
    
    // 采样策略
    int64_t sampleToken(const std::vector<float>& logits,
                       const GenerationParams& params);
    
    // 状态管理
    bool m_initialized = false;
    bool m_modelLoaded = false;
    std::atomic<bool> m_isGenerating{false};
    
    // ONNX Runtime 对象
    std::unique_ptr<Ort::Session> m_session;
    std::unique_ptr<Ort::Env> m_env;
    std::unique_ptr<Ort::MemoryInfo> m_memoryInfo;
    
    // 配置
    std::string m_modelPath;
    std::string m_executionProvider = "cpu";
    int m_numThreads = 4;
    int m_graphOptimizationLevel = 99;
    
    // Token 词汇表 (简化)
    QVector<QString> m_vocabulary;
    int m_eosTokenId = 2;
    int m_bosTokenId = 1;
    
    // 互斥锁
    mutable QMutex m_mutex;
};

} // namespace qt_ai_desktop
