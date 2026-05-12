#include "ChatWindow.h"
#include <qt-ai-desktop/ai_engine.h>
#include <qt-ai-desktop/backend_factory.h>
#include <qt-ai-desktop/logger.h>
#include <QVBoxLayout>
#include <QSettings>
#include <QMessageBox>
#include <QApplication>

ChatWindow::ChatWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_quickView(new QQuickView)
    , m_messageModel(new MessageModel(this))
{
    setupUI();
    setupConnections();
    loadSettings();
    
    // 默认使用 Simulation 后端
    m_engine = std::make_unique<qt_ai_desktop::AIEngine>();
    auto backend = qt_ai_desktop::BackendFactory::create("simulation");
    if (backend) {
        m_engine->setBackend(std::move(backend));
        m_currentBackend = "simulation";
        qt_ai_desktop::Logger::info("ChatWindow initialized with simulation backend");
    }
}

ChatWindow::~ChatWindow() {
    saveSettings();
}

MessageModel* ChatWindow::messageModel() const {
    return m_messageModel;
}

void ChatWindow::sendMessage(const QString& text) {
    if (text.trimmed().isEmpty()) return;
    
    // 添加用户消息到模型
    m_messageModel->addMessage(text, MessageModel::UserRole);
    
    // 创建 AI 回复占位
    m_messageModel->addMessage("", MessageModel::AssistantRole);
    
    // 流式生成回复
    m_engine->generateStream(text, [this](const QString& token) {
        QMetaObject::invokeMethod(this, [this, token]() {
            m_messageModel->appendToken(token);
            emit tokenReceived(token);
        });
    });
}

void ChatWindow::clearHistory() {
    m_messageModel->clear();
    qt_ai_desktop::Logger::info("Chat history cleared");
}

void ChatWindow::switchBackend(const QString& backendName) {
    auto backend = qt_ai_desktop::BackendFactory::create(backendName.toStdString());
    if (backend) {
        m_engine->setBackend(std::move(backend));
        m_currentBackend = backendName;
        qt_ai_desktop::Logger::info("Switched to backend: {}", backendName.toStdString());
    } else {
        emit errorOccurred(tr("Failed to load backend: %1").arg(backendName));
    }
}

void ChatWindow::loadModel(const QString& modelPath) {
    if (m_engine->setModel(modelPath.toStdString())) {
        qt_ai_desktop::Logger::info("Model loaded: {}", modelPath.toStdString());
    } else {
        emit errorOccurred(tr("Failed to load model: %1").arg(modelPath));
    }
}

void ChatWindow::setupUI() {
    // 设置 QML 上下文
    QQmlContext* context = m_quickView->rootContext();
    context->setContextProperty("chatWindow", this);
    context->setContextProperty("messageModel", m_messageModel);
    
    // 加载 QML
    m_quickView->setSource(QUrl("qrc:/qml/main.qml"));
    
    // 设置为主窗口
    QWidget* container = QWidget::createWindowContainer(m_quickView, this);
    setCentralWidget(container);
    setWindowTitle(tr("Qt AI Desktop - Chat"));
    resize(800, 600);
}

void ChatWindow::setupConnections() {
    connect(this, &ChatWindow::errorOccurred, this, [this](const QString& error) {
        QMessageBox::warning(this, tr("Error"), error);
    });
}

void ChatWindow::loadSettings() {
    QSettings settings;
    restoreGeometry(settings.value("chatWindow/geometry").toByteArray());
}

void ChatWindow::saveSettings() {
    QSettings settings;
    settings.setValue("chatWindow/geometry", saveGeometry());
}
