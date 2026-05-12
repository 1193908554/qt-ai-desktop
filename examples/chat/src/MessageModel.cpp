#include "MessageModel.h"

MessageModel::MessageModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int MessageModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_messages.size());
}

QVariant MessageModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(m_messages.size()))
        return {};

    const auto& message = m_messages[index.row()];

    switch (role) {
    case ContentRole:
        return message.content;
    case TimestampRole:
        return message.timestamp;
    case SenderRole:
        return message.sender;
    case IsStreamingRole:
        return message.isStreaming;
    default:
        return {};
    }
}

QHash<int, QByteArray> MessageModel::roleNames() const {
    return {
        {ContentRole, "content"},
        {TimestampRole, "timestamp"},
        {SenderRole, "sender"},
        {IsStreamingRole, "isStreaming"}
    };
}

int MessageModel::count() const {
    return static_cast<int>(m_messages.size());
}

void MessageModel::addMessage(const QString& content, Sender sender) {
    beginInsertRows(QModelIndex(), m_messages.size(), m_messages.size());
    m_messages.push_back({
        content,
        QDateTime::currentDateTime(),
        sender,
        true
    });
    endInsertRows();
    emit countChanged();
    emit messageAdded(m_messages.size() - 1);
}

void MessageModel::appendToken(const QString& token) {
    if (m_messages.empty()) return;

    auto& lastMessage = m_messages.back();
    if (lastMessage.sender != AssistantRole) return;

    lastMessage.content += token;

    QModelIndex lastIndex = index(m_messages.size() - 1);
    emit dataChanged(lastIndex, lastIndex, {ContentRole});
}

void MessageModel::clear() {
    beginResetModel();
    m_messages.clear();
    endResetModel();
    emit countChanged();
}

void MessageModel::finishStreaming() {
    if (m_messages.empty()) return;

    auto& lastMessage = m_messages.back();
    lastMessage.isStreaming = false;

    QModelIndex lastIndex = index(m_messages.size() - 1);
    emit dataChanged(lastIndex, lastIndex, {IsStreamingRole});
}
