#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <memory>

namespace QtAIDesktop {

/**
 * @brief AI 推理引擎接口
 * 
 * 提供统一的 AI 推理接口，支持多种后端实现
 */
class AIEngine : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isLoaded READ isLoaded NOTIFY isLoadedChanged)
    Q_PROPERTY(QString modelName READ modelName NOTIFY modelNameChanged)
    Q_PROPERTY(int tokenCount READ tokenCount NOTIFY tokenCountChanged)

public:
    explicit AIEngine(QObject *parent = nullptr);
    ~AIEngine();

    // 属性访问器
    bool isLoaded() const;
    QString modelName() const;
    int tokenCount() const;

    /**
     * @brief 加载模型
     * @param modelPath 模型文件路径
     * @return 是否成功
     */
    Q_INVOKABLE bool loadModel(const QString &modelPath);

    /**
     * @brief 卸载模型
     */
    Q_INVOKABLE void unloadModel();

    /**
     * @brief 发送聊天请求
     * @param prompt 输入提示
     * @return 模型响应
     */
    Q_INVOKABLE QString chat(const QString &prompt);

    /**
     * @brief 发送聊天请求（异步）
     * @param prompt 输入提示
     */
    Q_INVOKABLE void chatAsync(const QString &prompt);

    /**
     * @brief 停止当前生成
     */
    Q_INVOKABLE void stop();

    /**
     * @brief 设置生成参数
     * @param params 参数映射
     */
    Q_INVOKABLE void setParameters(const QVariantMap &params);

signals:
    /**
     * @brief 流式响应信号
     * @param token 新生成的 token
     */
    void tokenGenerated(const QString &token);

    /**
     * @brief 生成完成信号
     * @param response 完整响应
     */
    void responseComplete(const QString &response);

    /**
     * @brief 错误信号
     * @param error 错误信息
     */
    void errorOccurred(const QString &error);

    /**
     * @brief 状态变化信号
     */
    void isLoadedChanged();
    void modelNameChanged();
    void tokenCountChanged();

protected:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace QtAIDesktop