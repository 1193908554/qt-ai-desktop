#include "qt-ai-desktop/simulation_backend.h"
#include <QThread>
#include <QCoreApplication>
#include <QDebug>
#include <chrono>

namespace QtAIDesktop {

// ============================================================================
// 注册模拟后端
// ============================================================================
QTAI_REGISTER_BACKEND(SimulationBackend, "simulation")

// ============================================================================
// 实现
// ============================================================================

SimulationBackend::~SimulationBackend() {
    shutdown();
}

bool SimulationBackend::initialize(const QVariantMap &options) {
    Q_UNUSED(options);
    
    if (m_initialized) {
        return true;
    }
    
    m_initialized = true;
    qDebug() << "[Simulation] Backend initialized";
    return true;
}

void SimulationBackend::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    unloadModel();
    m_initialized = false;
    qDebug() << "[Simulation] Backend shutdown";
}

LoadResult SimulationBackend::loadModel(const QString &modelPath, 
                                         const QVariantMap &options) {
    Q_UNUSED(options);
    
    LoadResult result;
    
    if (!m_initialized) {
        result.error = "Backend not initialized";
        return result;
    }
    
    if (m_modelLoaded) {
        unloadModel();
    }
    
    qDebug() << "[Simulation] Loading model:" << modelPath;
    
    // 模拟加载延迟
    QThread::msleep(100);
    
    m_modelLoaded = true;
    m_modelName = QFileInfo(modelPath).completeBaseName();
    
    result.success = true;
    result.modelSize = 1024 * 1024 * 100; // 模拟 100MB
    result.memoryRequired = 1024 * 1024 * 200; // 模拟 200MB
    result.contextLength = 2048;
    
    qDebug() << "[Simulation] Model loaded:" << m_modelName;
    
    return result;
}

void SimulationBackend::unloadModel() {
    if (!m_modelLoaded) {
        return;
    }
    
    m_modelLoaded = false;
    m_modelName.clear();
    
    qDebug() << "[Simulation] Model unloaded";
}

bool SimulationBackend::isModelLoaded() const {
    return m_modelLoaded;
}

QString SimulationBackend::loadedModelName() const {
    return m_modelName;
}

InferenceResult SimulationBackend::infer(const QString &prompt, 
                                          const QVariantMap &params) {
    InferenceResult result;
    
    if (!m_modelLoaded) {
        result.error = "Model not loaded";
        return result;
    }
    
    QElapsedTimer timer;
    timer.start();
    
    // 获取参数
    int maxTokens = params.value("maxTokens", 256).toInt();
    float temperature = params.value("temperature", 0.7f).toFloat();
    Q_UNUSED(temperature);
    
    // 选择一个模拟响应
    std::uniform_int_distribution<> dist(0, m_sampleResponses.size() - 1);
    QString response = m_sampleResponses[dist(m_rng)];
    
    // 限制 token 数
    QStringList words = response.split(' ');
    if (words.size() > maxTokens) {
        words = words.mid(0, maxTokens);
        response = words.join(' ') + "...";
    }
    
    // 模拟流式输出
    if (m_streamCallback) {
        const int charsPerChunk = 3;
        for (int i = 0; i < response.size() && !m_stopRequested; i += charsPerChunk) {
            QString chunk = response.mid(i, charsPerChunk);
            m_streamCallback(chunk);
            
            // 模拟处理延迟
            QThread::msleep(20);
            
            // 处理事件以保持响应性
            QCoreApplication::processEvents();
        }
    }
    
    qint64 elapsed = timer.elapsed();
    int tokens = response.split(' ').size();
    
    // 更新统计
    m_totalInferences++;
    m_totalTokens += tokens;
    m_totalTimeMs += elapsed;
    
    result.success = true;
    result.text = response;
    result.tokensGenerated = tokens;
    result.tokensPerSecond = tokens / (elapsed / 1000.0);
    result.durationMs = elapsed;
    
    qDebug() << "[Simulation] Inference complete:"
             << tokens << "tokens in" << elapsed << "ms"
             << "(" << result.tokensPerSecond << "tokens/s)";
    
    return result;
}

void SimulationBackend::setStreamCallback(std::function<void(const QString &)> callback) {
    m_streamCallback = std::move(callback);
}

void SimulationBackend::stopGeneration() {
    m_stopRequested = true;
}

QVariantMap SimulationBackend::statistics() const {
    QVariantMap stats;
    stats["backend"] = name();
    stats["version"] = version();
    stats["totalInferences"] = m_totalInferences;
    stats["totalTokens"] = m_totalTokens;
    stats["totalTimeMs"] = m_totalTimeMs;
    stats["avgTokensPerSecond"] = m_totalTimeMs > 0 
        ? m_totalTokens / (m_totalTimeMs / 1000.0) 
        : 0.0;
    stats["modelLoaded"] = m_modelLoaded;
    stats["modelName"] = m_modelName;
    return stats;
}

} // namespace QtAIDesktop
