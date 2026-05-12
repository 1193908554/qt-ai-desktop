#pragma once

#include <QMainWindow>
#include <QQuickView>
#include <QQmlContext>
#include <memory>

namespace qt_ai_desktop {
class AIEngine;
}

class MessageModel;

class ChatWindow : public QMainWindow {
    Q_OBJECT
    Q_PROPERTY(MessageModel* messageModel READ messageModel CONSTANT)

public:
    explicit ChatWindow(QWidget* parent = nullptr);
    ~ChatWindow();

    MessageModel* messageModel() const;

public slots:
    void sendMessage(const QString& text);
    void clearHistory();
    void switchBackend(const QString& backendName);
    void loadModel(const QString& modelPath);

signals:
    void responseReceived(const QString& response);
    void errorOccurred(const QString& error);
    void tokenReceived(const QString& token);

private:
    void setupUI();
    void setupConnections();
    void loadSettings();
    void saveSettings();

    QQuickView* m_quickView;
    MessageModel* m_messageModel;
    std::unique_ptr<qt_ai_desktop::AIEngine> m_engine;
    QString m_currentBackend;
};
