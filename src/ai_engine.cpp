#include "qt-ai-desktop/ai_engine.h"

#include <QMutex>
#include <QMutexLocker>
#include <QThread>
#include <QTimer>
#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>

namespace QtAIDesktop {

// ============================================================================
// 错误码定义
// ============================================================================
enum class ErrorCode {
    None = 0,
    ModelNotFound = 1001,
    ModelLoadFailed = 1002,
    ModelCorrupted = 1003,
    OutOfMemory = 1004,
    NotLoaded = 2001,
    AlreadyGenerating = 2002,
    GenerationFailed = 2003,
    BackendError = 3001,
    Unknown = 9999
};

// ============================================================================
// 生成参数
// ============================================================================
struct GenerateParams {
    int maxTokens = 2048;
    float temperature = 0.7f;
    float topP = 0.9f;
    int topK = 40;
    float repeatPenalty = 1.1f;
    int seed = -1;
};

// ============================================================================
// 模型信息
// ============================================================================
struct ModelData {
    QString path;
    QString name;
    qint64 size = 0;
    bool isLoaded = false;
    BackendType backend = BackendType::Auto;
};

// ============================================================================
// AIEngine::Impl - 私有实现
// ============================================================================
class AIEngine::Impl {
public:
    // 状态
    GenerateStatus status = GenerateStatus::Idle;
    BackendType backendType = BackendType::Auto;
    BackendType activeBackend = BackendType::LlamaCpp;
    
    // 模型数据
    ModelData model;
    
    // 生成参数
    GenerateParams params;
    
    // 生成状态
    QString currentPrompt;
    QVariantList currentContext;
    bool stopRequested = false;
    
    // 统计
    int totalTokensGenerated = 0;
    qreal progressValue = 0.0;
    
    // 同步
    QMutex mutex;
    QThread *workerThread = nullptr;
    
    // 方法
    void setStatus(GenerateStatus newStatus) {
        if (status != newStatus) {
            status = newStatus;
        }
    }
    
    // 模拟流式输出（实际应调用真正的推理引擎）
    QString simulateGeneration(const QString &prompt) {
        // 模拟响应
        QStringList responses = {
            QString("这是一个关于 \"%1\" 的回答。\n\n"
                   "Qt AI Desktop 框架旨在提供跨平台的 AI 推理能力，"
                   "支持多种后端包括 llama.cpp、ONNX Runtime 和 whisper.cpp。"
                   "\n\n该框架提供了统一的 API 接口，"
                   "让开发者可以轻松集成 AI 功能到桌面应用中。").arg(prompt.left(50)),
            
            QString("基于您的问题 \"%1\"，我来分析一下：\n\n"
                   "1. **架构设计** - 采用模块化设计，支持插件式后端\n"
                   "2. **性能优化** - 支持 GPU 加速和量化推理\n"
                   "3. **易于集成** - 提供 Qt Quick 组件和 QML 绑定\n\n"
                   "如需了解更多细节，请查看项目文档。").arg(prompt.left(50)),
            
            QString("感谢您的提问。\n\n"
                   "Qt AI Desktop 是一个强大的桌面 AI 框架，"
                   "它结合了 Qt 的跨平台能力和先进的 AI 推理技术。\n\n"
                   "**主要特点：**\n"
                   "- 🚀 高性能推理\n"
                   "- 🔌 插件式架构\n"
                   "- 🎨 原生 Qt 集成\n"
                   "- 📱 多平台支持")
        };
        
        return responses[QRandomGenerator::global()->bounded(responses.size())];
    }
    
    // 模拟逐token输出
    void simulateStreamingOutput(const QString &response, AIEngine *engine) {
        const int tokenDelayMs = 30;  // 每个token延迟
        const int charsPerToken = 2;  // 平均每token字符数
        
        for (int i = 0; i < response.size(); i += charsPerToken) {
            if (stopRequested) {
                break;
            }
            
            QString token = response.mid(i, charsPerToken);
            totalTokensGenerated++;
            
            emit engine->tokenGenerated(token);
            
            // 模拟处理延迟
            QThread::msleep(tokenDelayMs);
        }
    }
};

// ============================================================================
// AIEngine 实现
// ============================================================================

AIEngine::AIEngine(QObject *parent)
    : QObject(parent)
    , pImpl(std::make_unique<Impl>())
{
    qDebug() << "AIEngine created";
}

AIEngine::~AIEngine() {
    if (pImpl->status == GenerateStatus::Generating) {
        pImpl->stopRequested = true;
    }
    qDebug() << "AIEngine destroyed";
}

// ----------------------------------------------------------------------------
// 属性访问器
// ----------------------------------------------------------------------------

bool AIEngine::isLoaded() const {
    return pImpl->model.isLoaded;
}

QString AIEngine::modelName() const {
    if (pImpl->model.path.isEmpty()) {
        return QString();
    }
    QFileInfo info(pImpl->model.path);
    return info.completeBaseName();
}

int AIEngine::tokenCount() const {
    return pImpl->totalTokensGenerated;
}

GenerateStatus AIEngine::status() const {
    return pImpl->status;
}

BackendType AIEngine::backend() const {
    return pImpl->backendType;
}

void AIEngine::setBackend(BackendType type) {
    if (pImpl->backendType != type) {
        pImpl->backendType = type;
        emit backendChanged();
    }
}

qreal AIEngine::progress() const {
    return pImpl->progressValue;
}

// ----------------------------------------------------------------------------
// 模型加载/卸载
// ----------------------------------------------------------------------------

bool AIEngine::loadModel(const QString &modelPath) {
    QMutexLocker locker(&pImpl->mutex);
    
    if (pImpl->status == GenerateStatus::Loading) {
        emit errorOccurred(static_cast<int>(ErrorCode::AlreadyGenerating), 
                          "Already loading a model");
        return false;
    }
    
    // 检查文件是否存在
    QFileInfo fileInfo(modelPath);
    if (!fileInfo.exists() && !modelPath.startsWith("http")) {
        emit errorOccurred(static_cast<int>(ErrorCode::ModelNotFound),
                          QString("Model file not found: %1").arg(modelPath));
        return false;
    }
    
    // 开始加载
    pImpl->setStatus(GenerateStatus::Loading);
    pImpl->model.path = modelPath;
    pImpl->progressValue = 0.0;
    emit statusChanged();
    emit progressChanged(0.0);
    
    qDebug() << "Loading model:" << modelPath;
    
    // 模拟异步加载过程
    QTimer *loadTimer = new QTimer(this);
    connect(loadTimer, &QTimer::timeout, this, [this, loadTimer]() {
        static int progress = 0;
        progress += 10;
        
        if (progress >= 100) {
            loadTimer->stop();
            loadTimer->deleteLater();
            
            QMutexLocker locker(&pImpl->mutex);
            pImpl->model.isLoaded = true;
            pImpl->model.size = QFileInfo(pImpl->model.path).size();
            pImpl->setStatus(GenerateStatus::Ready);
            pImpl->progressValue = 1.0;
            
            qDebug() << "Model loaded successfully:" << modelName();
            
            emit statusChanged();
            emit progressChanged(1.0);
            emit loadProgress(pImpl->model.size, pImpl->model.size());
            progress = 0;
        } else {
            pImpl->progressValue = progress / 100.0;
            emit progressChanged(pImpl->progressValue);
            emit loadProgress(progress * 1024 * 1024, 100 * 1024 * 1024);
        }
    });
    loadTimer->start(100);
    
    return true;
}

void AIEngine::unloadModel() {
    QMutexLocker locker(&pImpl->mutex);
    
    if (pImpl->status == GenerateStatus::Generating) {
        pImpl->stopRequested = true;
    }
    
    pImpl->model.isLoaded = false;
    pImpl->model.path.clear();
    pImpl->model.size = 0;
    pImpl->totalTokensGenerated = 0;
    pImpl->setStatus(GenerateStatus::Idle);
    
    qDebug() << "Model unloaded";
    
    emit statusChanged();
}

// ----------------------------------------------------------------------------
// 聊天接口
// ----------------------------------------------------------------------------

QString AIEngine::chat(const QString &prompt, const QVariantList &context) {
    QMutexLocker locker(&pImpl->mutex);
    
    if (!pImpl->model.isLoaded) {
        emit errorOccurred(static_cast<int>(ErrorCode::NotLoaded),
                          "Model not loaded. Call loadModel() first.");
        return QString();
    }
    
    if (pImpl->status == GenerateStatus::Generating) {
        emit errorOccurred(static_cast<int>(ErrorCode::AlreadyGenerating),
                          "Already generating. Call stop() first.");
        return QString();
    }
    
    pImpl->setStatus(GenerateStatus::Generating);
    pImpl->currentPrompt = prompt;
    pImpl->currentContext = context;
    pImpl->stopRequested = false;
    emit statusChanged();
    
    qDebug() << "Generating response for:" << prompt.left(50) << "...";
    
    QElapsedTimer timer;
    timer.start();
    
    // 模拟生成
    QString response = pImpl->simulateGeneration(prompt);
    
    int elapsed = timer.elapsed();
    qreal tokensPerSec = pImpl->totalTokensGenerated / (elapsed / 1000.0);
    
    pImpl->setStatus(GenerateStatus::Ready);
    emit statusChanged();
    
    // 构建统计信息
    QVariantMap stats;
    stats["tokens_per_sec"] = tokensPerSec;
    stats["total_tokens"] = pImpl->totalTokensGenerated;
    stats["duration_ms"] = elapsed;
    
    emit responseComplete(response, stats);
    
    return response;
}

void AIEngine::chatAsync(const QString &prompt, const QVariantList &context) {
    QMutexLocker locker(&pImpl->mutex);
    
    if (!pImpl->model.isLoaded) {
        emit errorOccurred(static_cast<int>(ErrorCode::NotLoaded),
                          "Model not loaded. Call loadModel() first.");
        return;
    }
    
    if (pImpl->status == GenerateStatus::Generating) {
        emit errorOccurred(static_cast<int>(ErrorCode::AlreadyGenerating),
                          "Already generating. Call stop() first.");
        return;
    }
    
    pImpl->setStatus(GenerateStatus::Generating);
    pImpl->currentPrompt = prompt;
    pImpl->currentContext = context;
    pImpl->stopRequested = false;
    emit statusChanged();
    
    qDebug() << "Async generation started for:" << prompt.left(50) << "...";
    
    // 在后台线程执行生成
    QThread *thread = QThread::create([this, prompt]() {
        QElapsedTimer timer;
        timer.start();
        
        // 模拟生成响应
        QString response = pImpl->simulateGeneration(prompt);
        
        // 模拟流式输出
        pImpl->simulateStreamingOutput(response, this);
        
        int elapsed = timer.elapsed();
        qreal tokensPerSec = pImpl->totalTokensGenerated / (elapsed / 1000.0);
        
        // 回到主线程
        QTimer::singleShot(0, this, [this, response, elapsed, tokensPerSec]() {
            pImpl->setStatus(GenerateStatus::Ready);
            emit statusChanged();
            
            QVariantMap stats;
            stats["tokens_per_sec"] = tokensPerSec;
            stats["total_tokens"] = pImpl->totalTokensGenerated;
            stats["duration_ms"] = elapsed;
            
            emit responseComplete(response, stats);
        });
    });
    
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

void AIEngine::stop() {
    QMutexLocker locker(&pImpl->mutex);
    
    if (pImpl->status == GenerateStatus::Generating) {
        pImpl->stopRequested = true;
        pImpl->setStatus(GenerateStatus::Stopping);
        emit statusChanged();
        
        qDebug() << "Generation stop requested";
    }
}

// ----------------------------------------------------------------------------
// 参数管理
// ----------------------------------------------------------------------------

void AIEngine::setParameters(const QVariantMap &params) {
    QMutexLocker locker(&pImpl->mutex);
    
    if (params.contains("maxTokens")) {
        pImpl->params.maxTokens = params["maxTokens"].toInt();
    }
    if (params.contains("temperature")) {
        float temp = params["temperature"].toFloat();
        pImpl->params.temperature = qBound(0.0f, temp, 2.0f);
    }
    if (params.contains("topP")) {
        float topP = params["topP"].toFloat();
        pImpl->params.topP = qBound(0.0f, topP, 1.0f);
    }
    if (params.contains("topK")) {
        int topK = params["topK"].toInt();
        pImpl->params.topK = qBound(1, topK, 100);
    }
    if (params.contains("repeatPenalty")) {
        float penalty = params["repeatPenalty"].toFloat();
        pImpl->params.repeatPenalty = qBound(1.0f, penalty, 2.0f);
    }
    if (params.contains("seed")) {
        pImpl->params.seed = params["seed"].toInt();
    }
    
    qDebug() << "Parameters updated:"
             << "temp=" << pImpl->params.temperature
             << "topP=" << pImpl->params.topP
             << "maxTokens=" << pImpl->params.maxTokens;
}

QVariantMap AIEngine::parameters() const {
    QMutexLocker locker(&pImpl->mutex);
    
    QVariantMap params;
    params["maxTokens"] = pImpl->params.maxTokens;
    params["temperature"] = pImpl->params.temperature;
    params["topP"] = pImpl->params.topP;
    params["topK"] = pImpl->params.topK;
    params["repeatPenalty"] = pImpl->params.repeatPenalty;
    params["seed"] = pImpl->params.seed;
    
    return params;
}

QVariantMap AIEngine::modelInfo() const {
    QMutexLocker locker(&pImpl->mutex);
    
    QVariantMap info;
    info["path"] = pImpl->model.path;
    info["name"] = modelName();
    info["size"] = pImpl->model.size;
    info["isLoaded"] = pImpl->model.isLoaded;
    info["backend"] = static_cast<int>(pImpl->activeBackend);
    info["tokenCount"] = pImpl->totalTokensGenerated;
    
    return info;
}

} // namespace QtAIDesktop
