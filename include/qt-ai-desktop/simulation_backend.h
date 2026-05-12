#pragma once

#include "qt-ai-desktop/backend.h"
#include <QMutex>
#include <QElapsedTimer>
#include <random>

namespace QtAIDesktop {

/**
 * @brief 模拟后端 - 用于测试和演示
 * 
 * 不依赖任何外部库，模拟 AI 推理过程。
 * 适用于单元测试、功能演示、UI 开发调试。
 */
class SimulationBackend : public IBackend {
public:
    SimulationBackend() = default;
    ~SimulationBackend() override;

    // IBackend interface
    QString name() const override { return "simulation"; }
    QString version() const override { return "1.0.0"; }
    QString description() const override { 
        return "Simulation backend for testing and demo purposes"; 
    }
    
    BackendCapabilities capabilities() const override {
        BackendCapabilities caps;
        caps.supportsChat = true;
        caps.supportsStreaming = true;
        caps.supportsGPU = false;
        caps.supportsEmbeddings = false;
        caps.supportsImageGeneration = false;
        caps.supportsSpeechRecognition = false;
        caps.supportedFormats = {"txt"};
        return caps;
    }

    bool initialize(const QVariantMap &options = {}) override;
    void shutdown() override;
    LoadResult loadModel(const QString &modelPath, 
                         const QVariantMap &options = {}) override;
    void unloadModel() override;
    bool isModelLoaded() const override;
    QString loadedModelName() const override;
    InferenceResult infer(const QString &prompt, 
                          const QVariantMap &params = {}) override;
    void setStreamCallback(std::function<void(const QString &)> callback) override;
    void stopGeneration() override;
    QVariantMap statistics() const override;

private:
    // 状态
    bool m_initialized = false;
    bool m_modelLoaded = false;
    bool m_stopRequested = false;
    QString m_modelName;
    
    // 统计
    int m_totalInferences = 0;
    int m_totalTokens = 0;
    qint64 m_totalTimeMs = 0;
    
    // 流式回调
    std::function<void(const QString &)> m_streamCallback;
    
    // 随机数生成
    std::mt19937 m_rng{std::random_device{}()};
    
    // 模拟响应数据
    QStringList m_sampleResponses = {
        "Based on my analysis of your question, here is a comprehensive response. "
        "The key points to consider are accuracy, performance, and maintainability. "
        "In modern C++ development, these aspects are crucial for building robust systems.",
        
        "This is an interesting topic. Let me break it down into several parts:\n\n"
        "1. **Architecture Design** - A well-designed architecture is the foundation "
        "of any successful project. It should be modular, extensible, and testable.\n\n"
        "2. **Performance Optimization** - Consider using techniques like move semantics, "
        "RAII, and zero-cost abstractions.\n\n"
        "3. **Error Handling** - Use Result types or exceptions consistently throughout "
        "your codebase.",
        
        "Here's a practical example of how to implement this pattern:\n\n"
        "```cpp\n"
        "auto result = engine->infer(prompt, {\"temperature\", 0.7});\n"
        "if (result.success) {\n"
        "    qDebug() << result.text;\n"
        "}\n"
        "```\n\n"
        "The key advantage of this approach is type safety and clear error propagation.",
        
        "Great question! Let me provide a detailed explanation.\n\n"
        "The Qt AI Desktop framework is designed with the following principles:\n"
        "- **Modularity**: Each component is independent and replaceable\n"
        "- **Type Safety**: Leverage C++20 concepts for compile-time checks\n"
        "- **Performance**: Minimal overhead with zero-cost abstractions\n"
        "- **Cross-platform**: Works on Windows, Linux, and macOS"
    };
};

} // namespace QtAIDesktop
