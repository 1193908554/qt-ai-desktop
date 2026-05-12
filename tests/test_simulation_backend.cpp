#include <gtest/gtest.h>
#include "qt-ai-desktop/simulation_backend.h"
#include "qt-ai-desktop/backend.h"

using namespace QtAIDesktop;

class SimulationBackendTest : public ::testing::Test {
protected:
    void SetUp() override {
        backend = std::make_unique<SimulationBackend>();
    }
    
    void TearDown() override {
        backend.reset();
    }
    
    std::unique_ptr<SimulationBackend> backend;
};

// ============================================================================
// 基本信息测试
// ============================================================================

TEST_F(SimulationBackendTest, Name) {
    EXPECT_EQ(backend->name(), "simulation");
}

TEST_F(SimulationBackendTest, Version) {
    EXPECT_FALSE(backend->version().isEmpty());
}

TEST_F(SimulationBackendTest, Description) {
    EXPECT_FALSE(backend->description().isEmpty());
}

// ============================================================================
// 能力测试
// ============================================================================

TEST_F(SimulationBackendTest, Capabilities) {
    BackendCapabilities caps = backend->capabilities();
    
    EXPECT_TRUE(caps.supportsChat);
    EXPECT_TRUE(caps.supportsStreaming);
    EXPECT_FALSE(caps.supportsGPU);
    EXPECT_FALSE(caps.supportsEmbeddings);
    EXPECT_FALSE(caps.supportsImageGeneration);
    EXPECT_FALSE(caps.supportsSpeechRecognition);
    EXPECT_FALSE(caps.supportedFormats.isEmpty());
}

// ============================================================================
// 初始化/关闭测试
// ============================================================================

TEST_F(SimulationBackendTest, Initialize) {
    EXPECT_TRUE(backend->initialize());
    EXPECT_TRUE(backend->initialize());  // 重复初始化应安全
}

TEST_F(SimulationBackendTest, Shutdown) {
    backend->initialize();
    backend->shutdown();
    // 关闭后不应崩溃
}

// ============================================================================
// 模型加载测试
// ============================================================================

TEST_F(SimulationBackendTest, LoadModel) {
    backend->initialize();
    
    LoadResult result = backend->loadModel("test-model.gguf");
    EXPECT_TRUE(result.success);
    EXPECT_FALSE(result.error.isEmpty() || result.error.isNull());
    EXPECT_TRUE(result.modelSize > 0);
    EXPECT_TRUE(result.memoryRequired > 0);
    EXPECT_TRUE(backend->isModelLoaded());
}

TEST_F(SimulationBackendTest, LoadModelBeforeInit) {
    // 未初始化时加载模型应返回错误
    LoadResult result = backend->loadModel("test.gguf");
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.error.isEmpty());
}

TEST_F(SimulationBackendTest, UnloadModel) {
    backend->initialize();
    backend->loadModel("test.gguf");
    EXPECT_TRUE(backend->isModelLoaded());
    
    backend->unloadModel();
    EXPECT_FALSE(backend->isModelLoaded());
}

TEST_F(SimulationBackendTest, ModelName) {
    backend->initialize();
    backend->loadModel("/path/to/my-model.gguf");
    
    EXPECT_FALSE(backend->loadedModelName().isEmpty());
}

// ============================================================================
// 推理测试
// ============================================================================

TEST_F(SimulationBackendTest, Infer) {
    backend->initialize();
    backend->loadModel("test.gguf");
    
    QVariantMap params;
    params["maxTokens"] = 100;
    params["temperature"] = 0.7;
    
    InferenceResult result = backend->infer("Hello, world!", params);
    
    EXPECT_TRUE(result.success);
    EXPECT_FALSE(result.text.isEmpty());
    EXPECT_TRUE(result.tokensGenerated > 0);
    EXPECT_TRUE(result.durationMs >= 0);
}

TEST_F(SimulationBackendTest, InferWithoutModel) {
    backend->initialize();
    
    InferenceResult result = backend->infer("test");
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.error.isEmpty());
}

TEST_F(SimulationBackendTest, StopGeneration) {
    backend->initialize();
    backend->loadModel("test.gguf");
    
    // 启动推理并立即停止
    backend->stopGeneration();
    
    QVariantMap params;
    params["maxTokens"] = 100;
    
    // 推理应该很快完成（因为停止请求）
    InferenceResult result = backend->infer("test", params);
    // 结果可能成功也可能被中断
}

// ============================================================================
// 流式回调测试
// ============================================================================

TEST_F(SimulationBackendTest, StreamCallback) {
    backend->initialize();
    backend->loadModel("test.gguf");
    
    int tokenCount = 0;
    backend->setStreamCallback([&tokenCount](const QString &token) {
        tokenCount++;
        EXPECT_FALSE(token.isEmpty());
    });
    
    QVariantMap params;
    params["maxTokens"] = 50;
    
    backend->infer("Tell me a story", params);
    EXPECT_GT(tokenCount, 0);
}

// ============================================================================
// 统计信息测试
// ============================================================================

TEST_F(SimulationBackendTest, Statistics) {
    backend->initialize();
    backend->loadModel("test.gguf");
    
    QVariantMap stats = backend->statistics();
    EXPECT_EQ(stats["backend"].toString(), "simulation");
    EXPECT_FALSE(stats["version"].toString().isEmpty());
    EXPECT_TRUE(stats["totalInferences"].toInt() >= 0);
    
    // 执行推理后统计应更新
    QVariantMap params;
    params["maxTokens"] = 50;
    backend->infer("test", params);
    
    stats = backend->statistics();
    EXPECT_GE(stats["totalInferences"].toInt(), 1);
}

// ============================================================================
// BackendFactory 测试
// ============================================================================

TEST(BackendFactoryTest, CreateSimulation) {
    auto backend = BackendFactory::create("simulation");
    ASSERT_NE(backend, nullptr);
    EXPECT_EQ(backend->name(), "simulation");
}

TEST(BackendFactoryTest, CreateUnknown) {
    auto backend = BackendFactory::create("unknown_backend");
    EXPECT_EQ(backend, nullptr);
}

TEST(BackendFactoryTest, AvailableBackends) {
    auto backends = BackendFactory::availableBackends();
    // 至少应该有 simulation 后端
    EXPECT_TRUE(backends.contains("simulation"));
}
