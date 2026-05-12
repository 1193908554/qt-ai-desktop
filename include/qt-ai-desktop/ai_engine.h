#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <QUrl>
#include <memory>

namespace QtAIDesktop {

/**
 * @brief AI 后端类型枚举
 */
enum class BackendType {
    Auto,       ///< 自动选择最佳后端
    LlamaCpp,   ///< llama.cpp 后端
    OnnxRuntime,///< ONNX Runtime 后端
    WhisperCpp, ///< whisper.cpp 后端（语音）
    Vulkan      ///< Vulkan Compute 后端
};

/**
 * @brief 生成状态枚举
 */
enum class GenerateStatus {
    Idle,       ///< 空闲
    Loading,    ///< 加载中
    Ready,      ///< 就绪
    Generating, ///< 生成中
    Stopping    ///< 停止中
};

/**
 * @brief AI 推理引擎接口
 * 
 * 提供统一的 AI 推理接口，支持多种后端实现。
 * 支持同步/异步推理、流式输出、多模态输入。
 * 
 * @example
 * @code
 * auto engine = new AIEngine(this);
 * connect(engine, &AIEngine::tokenGenerated, [](const QString &token) {
 *     qDebug() << token;
 * });
 * engine->loadModel("path/to/model.gguf");
 * engine->chatAsync("Hello, world!");
 * @endcode
 */
class AIEngine : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isLoaded READ isLoaded NOTIFY statusChanged)
    Q_PROPERTY(QString modelName READ modelName NOTIFY statusChanged)
    Q_PROPERTY(int tokenCount READ tokenCount NOTIFY statusChanged)
    Q_PROPERTY(GenerateStatus status READ status NOTIFY statusChanged)
    Q_PROPERTY(BackendType backend READ backend WRITE setBackend NOTIFY backendChanged)
    Q_PROPERTY(qreal progress READ progress NOTIFY progressChanged)

public:
    explicit AIEngine(QObject *parent = nullptr);
    ~AIEngine();

    // 属性访问器
    bool isLoaded() const;
    QString modelName() const;
    int tokenCount() const;
    GenerateStatus status() const;
    BackendType backend() const;
    void setBackend(BackendType type);
    qreal progress() const;

    /**
     * @brief 加载模型
     * @param modelPath 模型文件路径（本地路径或URL）
     * @return 是否成功开始加载（异步完成通过信号通知）
     */
    Q_INVOKABLE bool loadModel(const QString &modelPath);

    /**
     * @brief 卸载模型
     */
    Q_INVOKABLE void unloadModel();

    /**
     * @brief 发送聊天请求（同步）
     * @param prompt 输入提示
     * @param context 上下文消息列表 [{role: "user", content: "..."}]
     * @return 模型响应
     */
    Q_INVOKABLE QString chat(const QString &prompt, const QVariantList &context = {});

    /**
     * @brief 发送聊天请求（异步）
     * @param prompt 输入提示
     * @param context 上下文消息列表
     */
    Q_INVOKABLE void chatAsync(const QString &prompt, const QVariantList &context = {});

    /**
     * @brief 停止当前生成
     */
    Q_INVOKABLE void stop();

    /**
     * @brief 设置生成参数
     * @param params 参数映射，支持：
     *   - maxTokens (int): 最大生成token数，默认2048
     *   - temperature (float): 温度，默认0.7
     *   - topP (float): top-p采样，默认0.9
     *   - topK (int): top-k采样，默认40
     *   - repeatPenalty (float): 重复惩罚，默认1.1
     *   - seed (int): 随机种子，-1为随机
     */
    Q_INVOKABLE void setParameters(const QVariantMap &params);

    /**
     * @brief 获取当前参数
     */
    Q_INVOKABLE QVariantMap parameters() const;

    /**
     * @brief 获取模型信息
     */
    Q_INVOKABLE QVariantMap modelInfo() const;

signals:
    /**
     * @brief 流式响应信号（逐token）
     * @param token 新生成的 token
     */
    void tokenGenerated(const QString &token);

    /**
     * @brief 生成完成信号
     * @param response 完整响应
     * @param stats 生成统计 {tokens_per_sec, total_tokens, duration_ms}
     */
    void responseComplete(const QString &response, const QVariantMap &stats = {});

    /**
     * @brief 错误信号
     * @param code 错误码
     * @param message 错误信息
     */
    void errorOccurred(int code, const QString &message);

    /**
     * @brief 状态变化信号
     */
    void statusChanged();
    void backendChanged();
    void progressChanged(qreal progress);

    /**
     * @brief 模型加载进度
     * @param loaded 已加载大小
     * @param total 总大小
     */
    void loadProgress(qint64 loaded, qint64 total);

protected:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace QtAIDesktop
