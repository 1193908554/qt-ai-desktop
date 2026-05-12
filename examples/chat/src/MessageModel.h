#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QDateTime>
#include <vector>

class MessageModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        ContentRole = Qt::UserRole + 1,
        TimestampRole,
        SenderRole,
        IsStreamingRole
    };

    enum Sender {
        UserRole = 0,
        AssistantRole = 1,
        SystemRole = 2
    };
    Q_ENUM(Sender)

    struct Message {
        QString content;
        QDateTime timestamp;
        Sender sender;
        bool isStreaming;
    };

    explicit MessageModel(QObject* parent = nullptr);

    // QAbstractListModel 接口
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // 自定义方法
    int count() const;
    Q_INVOKABLE void addMessage(const QString& content, Sender sender);
    Q_INVOKABLE void appendToken(const QString& token);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void finishStreaming();

signals:
    void countChanged();
    void messageAdded(int index);

private:
    std::vector<Message> m_messages;
};
