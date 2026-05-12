#include "qt-ai-desktop/ai_chat_model.h"
#include "qt-ai-desktop/ai_engine.h"

#include <QDateTime>
#include <QDebug>

namespace QtAIDesktop {

// ============================================================================
// AIChatModel::Impl
// ============================================================================
class AIChatModel::Impl {
public:
    QList<ChatMessage> messages;
    AIEngine *engine = nullptr;
    int maxMessages = 100;
    int streamingIndex = -1;  // 当前正在流式输出的消息索引
};

// ============================================================================
// AIChatModel 实现
// ============================================================================

AIChatModel::AIChatModel(QObject *parent)
    : QAbstractListModel(parent)
    , pImpl(std::make_unique<Impl>())
{
}

AIChatModel::~AIChatModel() = default;

// ----------------------------------------------------------------------------
// QAbstractListModel interface
// ----------------------------------------------------------------------------

int AIChatModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return pImpl->messages.size();
}

QVariant AIChatModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= pImpl->messages.size()) {
        return QVariant();
    }

    const ChatMessage &msg = pImpl->messages[index.row()];

    switch (role) {
    case RoleContent:
        return msg.content;
    case RoleRole:
        return static_cast<int>(msg.role);
    case RoleTimestamp:
        return msg.timestamp;
    case RoleIsStreaming:
        return msg.isStreaming;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> AIChatModel::roleNames() const {
    return {
        {RoleContent, "content"},
        {RoleRole, "role"},
        {RoleTimestamp, "timestamp"},
        {RoleIsStreaming, "isStreaming"}
    };
}

// ----------------------------------------------------------------------------
// 属性
// ----------------------------------------------------------------------------

int AIChatModel::count() const {
    return pImpl->messages.size();
}

int AIChatModel::maxMessages() const {
    return pImpl->maxMessages;
}

void AIChatModel::setMaxMessages(int max) {
    if (pImpl->maxMessages != max) {
        pImpl->maxMessages = max;
        emit maxMessagesChanged();
        trimMessages();
    }
}

AIEngine* AIChatModel::engine() const {
    return pImpl->engine;
}

void AIChatModel::setEngine(AIEngine *engine) {
    if (pImpl->engine != engine) {
        // 断开旧引擎信号
        if (pImpl->engine) {
            disconnect(pImpl->engine, nullptr, this, nullptr);
        }

        pImpl->engine = engine;

        // 连接新引擎信号
        if (pImpl->engine) {
            connect(pImpl->engine, &AIEngine::tokenGenerated,
                    this, &AIChatModel::onTokenGenerated);
            connect(pImpl->engine, &AIEngine::responseComplete,
                    this, &AIChatModel::onResponseComplete);
            connect(pImpl->engine, &AIEngine::errorOccurred,
                    this, &AIChatModel::onErrorOccurred);
        }

        emit engineChanged();
    }
}

bool AIChatModel::isGenerating() const {
    return pImpl->streamingIndex >= 0;
}

// ----------------------------------------------------------------------------
// 公共方法
// ----------------------------------------------------------------------------

void AIChatModel::sendMessage(const QString &content) {
    if (content.isEmpty()) {
        return;
    }

    if (!pImpl->engine) {
        qWarning() << "AIChatModel: No engine set";
        return;
    }

    if (pImpl->engine->status() != GenerateStatus::Ready) {
        qWarning() << "AIChatModel: Engine not ready";
        return;
    }

    // 添加用户消息
    addMessage(MessageRole::User, content);

    // 构建上下文并发送异步请求
    QVariantList context = buildContext();
    pImpl->engine->chatAsync(content, context);
}

void AIChatModel::clear() {
    if (pImpl->messages.isEmpty()) {
        return;
    }

    beginResetModel();
    pImpl->messages.clear();
    pImpl->streamingIndex = -1;
    endResetModel();

    emit countChanged();
}

QVariantMap AIChatModel::getMessage(int index) const {
    if (index < 0 || index >= pImpl->messages.size()) {
        return QVariantMap();
    }

    const ChatMessage &msg = pImpl->messages[index];
    QVariantMap map;
    map["content"] = msg.content;
    map["role"] = static_cast<int>(msg.role);
    map["timestamp"] = msg.timestamp;
    map["isStreaming"] = msg.isStreaming;
    return map;
}

void AIChatModel::removeMessage(int index) {
    if (index < 0 || index >= pImpl->messages.size()) {
        return;
    }

    beginRemoveRows(QModelIndex(), index, index);
    pImpl->messages.removeAt(index);
    endRemoveRows();

    emit countChanged();
}

QVariantList AIChatModel::getHistory() const {
    QVariantList history;
    for (const auto &msg : pImpl->messages) {
        QVariantMap map;
        map["content"] = msg.content;
        map["role"] = static_cast<int>(msg.role);
        map["timestamp"] = msg.timestamp;
        history.append(map);
    }
    return history;
}

// ----------------------------------------------------------------------------
// 私有方法
// ----------------------------------------------------------------------------

void AIChatModel::addMessage(MessageRole role, const QString &content) {
    // 检查是否需要删除旧消息
    trimMessages();

    ChatMessage msg;
    msg.role = role;
    msg.content = content;
    msg.timestamp = QDateTime::currentDateTime();
    msg.isStreaming = (role == MessageRole::Assistant);

    beginInsertRows(QModelIndex(), pImpl->messages.size(), pImpl->messages.size());
    pImpl->messages.append(msg);

    if (role == MessageRole::Assistant) {
        pImpl->streamingIndex = pImpl->messages.size() - 1;
    }

    endInsertRows();

    emit countChanged();
    emit messageAdded(getMessage(pImpl->messages.size() - 1));
}

void AIChatModel::trimMessages() {
    while (pImpl->messages.size() > pImpl->maxMessages) {
        beginRemoveRows(QModelIndex(), 0, 0);
        pImpl->messages.removeFirst();
        endRemoveRows();
    }
}

QVariantList AIChatModel::buildContext() const {
    QVariantList context;
    
    // 限制上下文长度（保留最近的消息）
    int maxContext = qMin(pImpl->messages.size(), 20);
    int start = pImpl->messages.size() - maxContext;
    
    for (int i = start; i < pImpl->messages.size(); ++i) {
        const auto &msg = pImpl->messages[i];
        
        // 跳过正在流式输出的消息
        if (msg.isStreaming) {
            continue;
        }
        
        QVariantMap ctxMsg;
        ctxMsg["role"] = static_cast<int>(msg.role);
        ctxMsg["content"] = msg.content;
        context.append(ctxMsg);
    }
    
    return context;
}

// ----------------------------------------------------------------------------
// 信号槽
// ----------------------------------------------------------------------------

void AIChatModel::onTokenGenerated(const QString &token) {
    if (pImpl->streamingIndex < 0 || 
        pImpl->streamingIndex >= pImpl->messages.size()) {
        return;
    }

    // 追加 token 到当前消息
    pImpl->messages[pImpl->streamingIndex].content += token;

    // 通知视图更新
    QModelIndex index = createIndex(pImpl->streamingIndex, 0);
    emit dataChanged(index, index, {RoleContent});
    
    emit messageUpdated(pImpl->streamingIndex, 
                        pImpl->messages[pImpl->streamingIndex].content);
}

void AIChatModel::onResponseComplete(const QString &response, const QVariantMap &stats) {
    if (pImpl->streamingIndex >= 0 && 
        pImpl->streamingIndex < pImpl->messages.size()) {
        
        // 标记流式输出完成
        pImpl->messages[pImpl->streamingIndex].isStreaming = false;
        
        QModelIndex index = createIndex(pImpl->streamingIndex, 0);
        emit dataChanged(index, index, {RoleIsStreaming});
        
        pImpl->streamingIndex = -1;
        
        emit isGeneratingChanged();
    }
}

void AIChatModel::onErrorOccurred(int code, const QString &message) {
    Q_UNUSED(code);
    
    // 如果正在流式输出，移除未完成的消息
    if (pImpl->streamingIndex >= 0) {
        beginRemoveRows(QModelIndex(), pImpl->streamingIndex, pImpl->streamingIndex);
        pImpl->messages.removeAt(pImpl->streamingIndex);
        endRemoveRows();
        
        pImpl->streamingIndex = -1;
        emit isGeneratingChanged();
    }
    
    // 添加错误消息
    addMessage(MessageRole::System, QString("⚠️ Error: %1").arg(message));
}

} // namespace QtAIDesktop
