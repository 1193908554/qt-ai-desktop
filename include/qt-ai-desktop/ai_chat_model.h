#pragma once

#include <QAbstractListModel>
#include <QObject>
#include <QString>
#include <QVariantList>
#include <memory>

namespace QtAIDesktop {

class AIEngine;

/**
 * @brief 聊天消息角色
 */
enum class MessageRole {
    System,     ///< 系统消息
    User,       ///< 用户消息
    Assistant,  ///< AI 助手消息
    Function    ///< 函数调用
};

/**
 * @brief 聊天消息结构
 */
struct ChatMessage {
    MessageRole role = MessageRole::User;
    QString content;
    QDateTime timestamp;
    bool isStreaming = false;  ///< 是否正在流式输出中
};

/**
 * @brief 聊天数据模型
 * 
 * 用于 Qt Quick/QML 的聊天界面数据绑定。
 * 支持流式更新、消息管理、上下文窗口。
 * 
 * @example QML
 * @qml
 * import QtAIDesktop
 * 
 * AIChatModel {
 *     id: chatModel
 *     engine: aiEngine
 *     maxMessages: 50
 *     
 *     onMessageAdded: function(message) {
 *         console.log("New message:", message.content)
 *     }
 * }
 * 
 * ListView {
 *     model: chatModel
 *     delegate: Text { text: model.content }
 * }
 * @endqml
 */
class AIChatModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int maxMessages READ maxMessages WRITE setMaxMessages NOTIFY maxMessagesChanged)
    Q_PROPERTY(AIEngine* engine READ engine WRITE setEngine NOTIFY engineChanged)
    Q_PROPERTY(bool isGenerating READ isGenerating NOTIFY isGeneratingChanged)

public:
    // 自定义角色
    enum Roles {
        RoleContent = Qt::UserRole + 1,
        RoleRole,
        RoleTimestamp,
        RoleIsStreaming
    };

    explicit AIChatModel(QObject *parent = nullptr);
    ~AIChatModel();

    // QAbstractListModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // 属性
    int count() const;
    int maxMessages() const;
    void setMaxMessages(int max);
    AIEngine* engine() const;
    void setEngine(AIEngine *engine);
    bool isGenerating() const;

    /**
     * @brief 发送用户消息
     * @param content 消息内容
     */
    Q_INVOKABLE void sendMessage(const QString &content);

    /**
     * @brief 清空聊天记录
     */
    Q_INVOKABLE void clear();

    /**
     * @brief 获取消息
     * @param index 消息索引
     * @return 消息内容
     */
    Q_INVOKABLE QVariantMap getMessage(int index) const;

    /**
     * @brief 删除消息
     * @param index 消息索引
     */
    Q_INVOKABLE void removeMessage(int index);

    /**
     * @brief 获取聊天历史（用于导出）
     * @return 消息列表
     */
    Q_INVOKABLE QVariantList getHistory() const;

signals:
    void countChanged();
    void maxMessagesChanged();
    void engineChanged();
    void isGeneratingChanged();
    
    /**
     * @brief 新消息添加
     * @param message 消息内容
     */
    void messageAdded(const QVariantMap &message);
    
    /**
     * @brief 消息更新（流式输出时）
     * @param index 消息索引
     * @param content 新内容
     */
    void messageUpdated(int index, const QString &content);

private slots:
    void onTokenGenerated(const QString &token);
    void onResponseComplete(const QString &response, const QVariantMap &stats);
    void onErrorOccurred(int code, const QString &message);

private:
    void addMessage(MessageRole role, const QString &content);
    void trimMessages();
    QVariantList buildContext() const;

    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace QtAIDesktop
