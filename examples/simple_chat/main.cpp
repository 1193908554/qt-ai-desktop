#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>

#include "qt-ai-desktop/ai_engine.h"
#include "qt-ai-desktop/model_manager.h"
#include "qt-ai-desktop/ai_chat_model.h"

using namespace QtAIDesktop;

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("AI Chat Demo");
    app.setOrganizationName("Qt AI Desktop");
    app.setApplicationVersion("1.0.0");
    
    // 设置应用图标
    app.setWindowIcon(QIcon(":/qt-ai-desktop/icon.png"));
    
    // 创建核心对象
    AIEngine engine;
    ModelManager modelManager;
    AIChatModel chatModel;
    
    // 连接聊天模型和引擎
    chatModel.setEngine(&engine);
    
    // 创建 QML 引擎
    QQmlApplicationEngine qmlEngine;
    
    // 暴露对象到 QML
    qmlEngine.rootContext()->setContextProperty("aiEngine", &engine);
    qmlEngine.rootContext()->setContextProperty("modelManager", &modelManager);
    qmlEngine.rootContext()->setContextProperty("chatModel", &chatModel);
    
    // 扫描本地模型
    modelManager.scanLocalModels();
    
    // 加载 QML UI
    qmlEngine.load(QUrl("qrc:/qt-ai-desktop/examples/simple_chat/main.qml"));
    
    if (qmlEngine.rootObjects().isEmpty()) {
        qWarning() << "Failed to load QML file";
        return -1;
    }
    
    return app.exec();
}
