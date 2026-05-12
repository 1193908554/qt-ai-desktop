#pragma once

#include "qt-ai-desktop/backend.h"
#include <QMutex>
#include <QElapsedTimer>
#include <QVector>
#include <functional>
#include <memory>

// 前向声明 llama.cpp 类型
struct llama_model;
struct llama_context;
struct llama_sampler;

namespace QtAIDesktop {

/**
 * @brief llama.cpp 后端配置
 */
struct LlamaConfig {
    int contextLength = 2048;       ///< 上下文长度
    int batchSize = 512;            ///< 批处理大小
    int gpuLayers = 99;             ///< GPU 层数（99=全部）
    bool useMmap = true;            ///< 使用内存映射
    bool useMlock = false;          ///< 锁定内存
    int threads = 4;                ///< 推理线程数
    bool flashAttention = false;    ///< Flash Attention
    std::string ropeFreqBase;       ///< RoPE 频率基数
    std::string ropeFreqScale;      ///< RoPE 频率缩放
};

/**
 * @brief 生成参数
 */
struct LlamaGenerateParams {
    int maxTokens = 256;            ///< 最大生成 token 数
    float temperature = 0.7f;       ///< 温度
    float topP = 0.9f;             ///< Top-P 采样
    int topK = 40;                  ///< Top-K 采样
    float repeatPenalty = 1.1f;     ///< 重复惩罚
    int repeatLastN = 64;           ///< 重复惩罚窗口
    float mirostatTau = 5.0f;      ///< Mirostat tau
    float mirostatEta = 0.1f;      ///< Mirostat eta
    int seed = -1;                  ///< 随机种子
    std::string stopSequence;       ///< 停止序列
};

/**
 * @brief llama.cpp 后端实现
 * 
 * 基于 llama.cpp 的 GGUF 模型推理后端。
 * 支持：
 * - GGUF 格式模型加载
 * - 流式推理输出
 * - GPU 加速（CUDA/Metal/Vulkan）
 * - 多种量化格式（Q4_0, Q4_K_M, Q5_K_M, Q8_0 等）
 * - Mirostat 采样
 * 
 * @example
 * @code
 * auto backend = BackendFactory::create("llamacpp");
 * backend->initialize({{"gpuLayers", 32}});
 * backend->loadModel("model.gguf");
 * 
 * backend->setStreamCallback([](const QString &token) {
 *     qDebug() << token;
 * });
 * 
 * auto result = backend->infer("Hello, world!");
 * @endcode
 */
class LlamaCppBackend : public IBackend {
public:
    LlamaCppBackend();
    ~LlamaCppBackend() override;

    // IBackend 接口
    QString name() const override { return "llamacpp"; }
    QString version() const override;
    QString description() const override {
        return "GGUF model inference via llama.cpp";
    }
    
    BackendCapabilities capabilities() const override {
        BackendCapabilities caps;
        caps.supportsChat = true;
        caps.supportsStreaming = true;
        caps.supportsGPU = true;
        caps.supportsEmbeddings = true;
        caps.supportsImageGeneration = false;
        caps.supportsSpeechRecognition = false;
        caps.supportedFormats = {"gguf", "ggml"};
        return caps;
    }

    bool initialize(const QVariantMap &options = {}) override;
    void shutdown() override;
    
    LoadResult loadModel(const QString &modelPath, 
                         const QVariantMap &options = {}) override;
    void unloadModel() override;
    bool isModelLoaded() const override;
    QString loadedModelName() const override;
    
    InferenceResult infer(const QString &prompt, 
                          const QVariantMap &params = {}) override;
    
    void setStreamCallback(std::function<void(const QString &)> callback) override;
    void stopGeneration() override;
    
    QVariantMap statistics() const override;

    // llama.cpp 特有功能
    
    /**
     * @brief 设置系统提示
     */
    void setSystemPrompt(const QString &prompt);
    
    /**
     * @brief 获取模型详细信息
     */
    QVariantMap modelDetails() const;
    
    /**
     * @brief 评估嵌入向量
     */
    QVector<float> embeddings(const QString &text);

private:
    // 内部方法
    bool loadModelInternal(const QString &path);
    std::vector<int> tokenize(const QString &text, bool addBos = true);
    QString detokenize(const std::vector<int> &tokens);
    bool evaluateTokens(const std::vector<int> &tokens);
    int sampleToken();
    bool isStopToken(int token) const;
    
    // 配置
    LlamaConfig m_config;
    LlamaGenerateParams m_genParams;
    
    // llama.cpp 对象
    llama_model *m_model = nullptr;
    llama_context *m_ctx = nullptr;
    llama_sampler *m_sampler = nullptr;
    
    // 状态
    bool m_initialized = false;
    bool m_modelLoaded = false;
    bool m_stopRequested = false;
    QString m_modelName;
    QString m_modelPath;
    QString m_systemPrompt;
    
    // 流式回调
    std::function<void(const QString &)> m_streamCallback;
    
    // 统计
    int m_totalInferences = 0;
    int m_totalTokens = 0;
    qint64 m_totalTimeMs = 0;
    int m_contextSize = 0;
    
    // 同步
    mutable QMutex m_mutex;
};

} // namespace QtAIDesktop
