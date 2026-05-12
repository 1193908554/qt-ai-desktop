#include <QApplication>
#include <QQmlApplicationEngine>
#include "ChatWindow.h"
#include <qt-ai-desktop/logger.h>
#include <qt-ai-desktop/config.h>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Qt AI Desktop - Chat");
    app.setOrganizationName("QtAIDesktop");
    app.setApplicationVersion("1.0.0");

    // 初始化日志
    qt_ai_desktop::Logger::init("chat_app.log");
    qt_ai_desktop::Logger::info("Application starting...");

    // 加载配置
    qt_ai_desktop::Config config;
    config.load("config.json");

    // 创建主窗口
    ChatWindow window;
    window.show();

    // 处理命令行参数
    QStringList args = app.arguments();
    for (int i = 1; i < args.size(); ++i) {
        if (args[i] == "--model" && i + 1 < args.size()) {
            window.loadModel(args[++i]);
        } else if (args[i] == "--backend" && i + 1 < args.size()) {
            window.switchBackend(args[++i]);
        }
    }

    qt_ai_desktop::Logger::info("Application started successfully");

    return app.exec();
}
