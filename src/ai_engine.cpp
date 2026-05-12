#include "qt-ai-desktop/ai_engine.h"

#include <QMutex>
#include <QThread>
#include <QTimer>

namespace QtAIDesktop {

class AIEngine::Impl {
public:
    bool isLoaded = false;
    QString modelName;
    int tokenCount = 0;
    bool isGenerating = false;
    QMutex mutex;
    
    // 模型参数
    int maxTokens = 2048;
    float temperature = 0.7f;
    float topP = 0.9f;
};

AIEngine::AIEngine(QObject *parent)
    : QObject(parent)
    , pImpl(std::make_unique<Impl>())
{
}

AIEngine::~AIEngine() = default;

bool AIEngine::isLoaded() const {
    return pImpl->isLoaded;
}

QString AIEngine::modelName() const {
    return pImpl->modelName;
}

int AIEngine::tokenCount() const {
    return pImpl->tokenCount;
}

bool AIEngine::loadModel(const QString &modelPath) {
    QMutexLocker locker(&pImpl->mutex);
    
    // TODO: 实际加载模型（llama.cpp / ONNX Runtime）
    // 这里是占位实现
    
    pImpl->modelName = modelPath;
    pImpl->isLoaded = true;
    
    emit isLoadedChanged();
    emit modelNameChanged();
    
    return true;
}

void AIEngine::unloadModel() {
    QMutexLocker locker(&pImpl->mutex);
    
    pImpl->isLoaded = false;
    pImpl->modelName.clear();
    pImpl->tokenCount = 0;
    
    emit isLoadedChanged();
    emit modelNameChanged();
}

QString AIEngine::chat(const QString &prompt) {
    if (!pImpl->isLoaded) {
        emit errorOccurred("Model not loaded");
        return QString();
    }
    
    // TODO: 实际调用推理引擎
    // 这里是占位实现
    QString response = QString("AI response to: %1").arg(prompt);
    
    return response;
}

void AIEngine::chatAsync(const QString &prompt) {
    if (pImpl->isGenerating) {
        emit errorOccurred("Already generating");
        return;
    }
    
    if (!pImpl->isLoaded) {
        emit errorOccurred("Model not loaded");
        return;
    }
    
    pImpl->isGenerating = true;
    
    // TODO: 在后台线程执行推理
    // 这里是占位实现
    QTimer::singleShot(100, this, [this, prompt]() {
        QString response = chat(prompt);
        pImpl->isGenerating = false;
        emit responseComplete(response);
    });
}

void AIEngine::stop() {
    pImpl->isGenerating = false;
    // TODO: 停止推理
}

void AIEngine::setParameters(const QVariantMap &params) {
    QMutexLocker locker(&pImpl->mutex);
    
    if (params.contains("maxTokens")) {
        pImpl->maxTokens = params["maxTokens"].toInt();
    }
    if (params.contains("temperature")) {
        pImpl->temperature = params["temperature"].toFloat();
    }
    if (params.contains("topP")) {
        pImpl->topP = params["topP"].toFloat();
    }
}

} // namespace QtAIDesktop