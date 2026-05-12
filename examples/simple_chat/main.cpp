#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QTextEdit>
#include <qt-ai-desktop/ai_engine.h>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Simple Chat");
    app.setApplicationVersion("0.1.0");
    
    // 创建主窗口
    QWidget window;
    window.setWindowTitle("Qt AI Desktop - Simple Chat");
    window.setMinimumSize(400, 300);
    
    // 创建布局
    QVBoxLayout *layout = new QVBoxLayout(&window);
    
    // 聊天显示区域
    QTextEdit *chatDisplay = new QTextEdit(&window);
    chatDisplay->setReadOnly(true);
    layout->addWidget(chatDisplay);
    
    // 输入框和按钮
    QHBoxLayout *inputLayout = new QHBoxLayout();
    
    QLineEdit *inputField = new QLineEdit(&window);
    inputField->setPlaceholderText("输入消息...");
    inputLayout->addWidget(inputField);
    
    QPushButton *sendButton = new QPushButton("发送", &window);
    inputLayout->addWidget(sendButton);
    
    layout->addLayout(inputLayout);
    
    // 创建 AI 引擎
    QtAIDesktop::AIEngine engine;
    
    // 连接信号
    QObject::connect(&engine, &QtAIDesktop::AIEngine::tokenGenerated,
                     [chatDisplay](const QString &token) {
        chatDisplay->moveCursor(QTextCursor::End);
        chatDisplay->insertPlainText(token);
    });
    
    QObject::connect(&engine, &QtAIDesktop::AIEngine::responseComplete,
                     [chatDisplay](const QString &response) {
        chatDisplay->append("\n");
    });
    
    QObject::connect(&engine, &QtAIDesktop::AIEngine::errorOccurred,
                     [chatDisplay](const QString &error) {
        chatDisplay->append(QString("Error: %1").arg(error));
    });
    
    // 发送按钮点击
    QObject::connect(sendButton, &QPushButton::clicked, [&]() {
        QString input = inputField->text().trimmed();
        if (input.isEmpty()) return;
        
        chatDisplay->append(QString("You: %1").arg(input));
        inputField->clear();
        
        engine.chatAsync(input);
    });
    
    // 输入框回车
    QObject::connect(inputField, &QLineEdit::returnPressed, [sendButton]() {
        sendButton->click();
    });
    
    // 显示窗口
    window.show();
    
    return app.exec();
}