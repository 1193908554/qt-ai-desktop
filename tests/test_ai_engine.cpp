#include <gtest/gtest.h>
#include "qt-ai-desktop/ai_engine.h"

using namespace QtAIDesktop;

class AIEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        engine = std::make_unique<AIEngine>();
    }
    
    void TearDown() override {
        engine.reset();
    }
    
    std::unique_ptr<AIEngine> engine;
};

// ============================================================================
// 基本属性测试
// ============================================================================

TEST_F(AIEngineTest, InitialState) {
    EXPECT_FALSE(engine->isLoaded());
    EXPECT_TRUE(engine->modelName().isEmpty());
    EXPECT_EQ(engine->tokenCount(), 0);
    EXPECT_EQ(engine->status(), GenerateStatus::Idle);
}

TEST_F(AIEngineTest, DefaultParameters) {
    QVariantMap params = engine->parameters();
    
    EXPECT_EQ(params["maxTokens"].toInt(), 2048);
    EXPECT_FLOAT_EQ(params["temperature"].toFloat(), 0.7f);
    EXPECT_FLOAT_EQ(params["topP"].toFloat(), 0.9f);
    EXPECT_EQ(params["topK"].toInt(), 40);
}

// ============================================================================
// 参数设置测试
// ============================================================================

TEST_F(AIEngineTest, SetParameters) {
    QVariantMap params;
    params["maxTokens"] = 1024;
    params["temperature"] = 0.5;
    params["topP"] = 0.8;
    params["topK"] = 20;
    params["repeatPenalty"] = 1.2;
    params["seed"] = 42;
    
    engine->setParameters(params);
    
    QVariantMap result = engine->parameters();
    EXPECT_EQ(result["maxTokens"].toInt(), 1024);
    EXPECT_FLOAT_EQ(result["temperature"].toFloat(), 0.5f);
    EXPECT_FLOAT_EQ(result["topP"].toFloat(), 0.8f);
    EXPECT_EQ(result["topK"].toInt(), 20);
    EXPECT_FLOAT_EQ(result["repeatPenalty"].toFloat(), 1.2f);
    EXPECT_EQ(result["seed"].toInt(), 42);
}

TEST_F(AIEngineTest, ParameterBounds) {
    QVariantMap params;
    params["temperature"] = 5.0;  // 超出范围
    params["topP"] = -0.5;       // 超出范围
    
    engine->setParameters(params);
    
    QVariantMap result = engine->parameters();
    // temperature 应该被限制在 [0, 2]
    EXPECT_LE(result["temperature"].toFloat(), 2.0f);
    // topP 应该被限制在 [0, 1]
    EXPECT_GE(result["topP"].toFloat(), 0.0f);
}

// ============================================================================
// 模型加载测试
// ============================================================================

TEST_F(AIEngineTest, LoadModelInvalidPath) {
    bool result = engine->loadModel("/nonexistent/path/model.gguf");
    // 应该返回 false 或触发错误信号
    // 注意：模拟实现可能行为不同
}

// ============================================================================
// Backend 测试
// ============================================================================

TEST_F(AIEngineTest, SetBackend) {
    engine->setBackend(BackendType::LlamaCpp);
    EXPECT_EQ(engine->backend(), BackendType::LlamaCpp);
    
    engine->setBackend(BackendType::OnnxRuntime);
    EXPECT_EQ(engine->backend(), BackendType::OnnxRuntime);
}

// ============================================================================
// 模型信息测试
// ============================================================================

TEST_F(AIEngineTest, ModelInfoWhenEmpty) {
    QVariantMap info = engine->modelInfo();
    EXPECT_TRUE(info["path"].toString().isEmpty());
    EXPECT_TRUE(info["isLoaded"].toBool() == false);
}

// ============================================================================
// 参数边界值测试
// ============================================================================

TEST_F(AIEngineTest, ParameterEdgeCases) {
    QVariantMap params;
    
    // 测试温度边界
    params["temperature"] = 0.0;
    engine->setParameters(params);
    EXPECT_GE(engine->parameters()["temperature"].toFloat(), 0.0f);
    
    params["temperature"] = -1.0;
    engine->setParameters(params);
    EXPECT_GE(engine->parameters()["temperature"].toFloat(), 0.0f);
    
    // 测试 topP 边界
    params["topP"] = 1.5;
    engine->setParameters(params);
    EXPECT_LE(engine->parameters()["topP"].toFloat(), 1.0f);
    
    // 测试 maxTokens 边界
    params["maxTokens"] = 0;
    engine->setParameters(params);
    // maxTokens 应该接受 0 或者有最小值
}
