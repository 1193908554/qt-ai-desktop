#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <memory>

namespace QtAIDesktop {

/**
 * @brief 推理结果
 */
struct InferenceResult {
    bool success = false;
    QString text;
    QString error;
    int tokensGenerated = 0;
    double tokensPerSecond = 0.0;
    qint64 durationMs = 0;
};

/**
 * @brief 模型加载结果
 */
struct LoadResult {
    bool success = false;
    QString error;
    qint64 modelSize = 0;
    qint64 memoryRequired = 0;
    int contextLength = 0;
};

/**
 * @brief 后端能力描述
 */
struct BackendCapabilities {
    bool supportsChat = false;
    bool supportsEmbeddings = false;
    bool supportsImageGeneration = false;
    bool supportsSpeechRecognition = false;
    bool supportsStreaming = false;
    bool supportsGPU = false;
    QStringList supportedFormats;
};

/**
 * @brief AI 后端插件接口
 * 
 * 所有 AI 后端必须实现此接口。
 * 通过 BackendFactory 注册和创建后端实例。
 * 
 * @example
 * @code
 * class LlamaCppBackend : public IBackend {
 * public:
 *     QString name() const override { return "llama.cpp"; }
 *     LoadResult loadModel(const QString &path, const QVariantMap &opts) override {
 *         // ... 加载模型
 *     }
 *     InferenceResult infer(const QString &prompt, const QVariantMap &params) override {
 *         // ... 执行推理
 *     }
 * };
 * @endcode
 */
class IBackend {
public:
    virtual ~IBackend() = default;

    /**
     * @brief 后端名称
     */
    virtual QString name() const = 0;

    /**
     * @brief 后端版本
     */
    virtual QString version() const = 0;

    /**
     * @brief 后端描述
     */
    virtual QString description() const = 0;

    /**
     * @brief 获取后端能力
     */
    virtual BackendCapabilities capabilities() const = 0;

    /**
     * @brief 初始化后端
     * @return 是否成功
     */
    virtual bool initialize(const QVariantMap &options = {}) = 0;

    /**
     * @brief 释放后端资源
     */
    virtual void shutdown() = 0;

    /**
     * @brief 加载模型
     * @param modelPath 模型路径
     * @param options 加载选项
     * @return 加载结果
     */
    virtual LoadResult loadModel(const QString &modelPath, 
                                 const QVariantMap &options = {}) = 0;

    /**
     * @brief 卸载模型
     */
    virtual void unloadModel() = 0;

    /**
     * @brief 模型是否已加载
     */
    virtual bool isModelLoaded() const = 0;

    /**
     * @brief 获取已加载的模型名称
     */
    virtual QString loadedModelName() const = 0;

    /**
     * @brief 执行推理（同步）
     * @param prompt 输入提示
     * @param params 生成参数
     * @return 推理结果
     */
    virtual InferenceResult infer(const QString &prompt, 
                                  const QVariantMap &params = {}) = 0;

    /**
     * @brief 设置流式回调
     * @param callback 每个 token 的回调函数
     */
    virtual void setStreamCallback(std::function<void(const QString &)> callback) = 0;

    /**
     * @brief 停止当前生成
     */
    virtual void stopGeneration() = 0;

    /**
     * @brief 获取后端统计信息
     */
    virtual QVariantMap statistics() const = 0;
};

/**
 * @brief 后端工厂 - 创建和管理后端实例
 */
class BackendFactory {
public:
    /**
     * @brief 注册后端类型
     * @param name 后端名称
     * @param creator 创建函数
     */
    static void registerBackend(const QString &name, 
                                std::function<std::unique_ptr<IBackend>()> creator);

    /**
     * @brief 创建后端实例
     * @param name 后端名称
     * @return 后端实例
     */
    static std::unique_ptr<IBackend> create(const QString &name);

    /**
     * @brief 获取已注册的后端名称列表
     */
    static QStringList availableBackends();

    /**
     * @brief 获取所有可用后端及其能力
     */
    static QVariantMap backendCapabilities();

private:
    struct Registry;
    static Registry& registry();
};

/**
 * @brief 后端注册辅助宏
 */
#define QTAI_REGISTER_BACKEND(ClassName, BackendName) \
    static bool _qtai_registered_##ClassName = []() { \
        QtAIDesktop::BackendFactory::registerBackend( \
            BackendName, \
            []() -> std::unique_ptr<QtAIDesktop::IBackend> { \
                return std::make_unique<ClassName>(); \
            }); \
        return true; \
    }();

} // namespace QtAIDesktop
