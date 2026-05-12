#include <gtest/gtest.h>
#include "qt-ai-desktop/config.h"
#include <QTemporaryDir>

using namespace QtAIDesktop;

class ConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());
    }
    
    std::unique_ptr<QTemporaryDir> tempDir;
};

// ============================================================================
// 创建默认配置测试
// ============================================================================

TEST_F(ConfigTest, CreateDefault) {
    QString path = tempDir->filePath("config.json");
    EXPECT_TRUE(Config::createDefault(path));
    
    auto config = Config::load(path);
    ASSERT_NE(config, nullptr);
    
    // 验证默认值
    EXPECT_EQ(config->get<int>("engine/maxTokens"), 2048);
    EXPECT_FLOAT_EQ(config->get<double>("engine/temperature"), 0.7);
    EXPECT_EQ(config->get<QString>("ui/theme"), "auto");
}

// ============================================================================
// 加载配置测试
// ============================================================================

TEST_F(ConfigTest, LoadExisting) {
    QString path = tempDir->filePath("config.json");
    Config::createDefault(path);
    
    auto config = Config::load(path);
    ASSERT_NE(config, nullptr);
    EXPECT_EQ(config->filePath(), path);
}

TEST_F(ConfigTest, LoadNonexistent) {
    auto config = Config::load(tempDir->filePath("nonexistent.json"));
    EXPECT_EQ(config, nullptr);
}

// ============================================================================
// 读写测试
// ============================================================================

TEST_F(ConfigTest, SetGetValues) {
    QString path = tempDir->filePath("config.json");
    Config::createDefault(path);
    
    auto config = Config::load(path);
    ASSERT_NE(config, nullptr);
    
    // 设置值
    config->set("engine/maxTokens", 4096);
    config->set("engine/temperature", 0.9);
    config->set("custom/nested/value", "hello");
    
    // 读取值
    EXPECT_EQ(config->get<int>("engine/maxTokens"), 4096);
    EXPECT_FLOAT_EQ(config->get<double>("engine/temperature"), 0.9);
    EXPECT_EQ(config->get<QString>("custom/nested/value"), "hello");
}

TEST_F(ConfigTest, DefaultValue) {
    QString path = tempDir->filePath("config.json");
    Config::createDefault(path);
    
    auto config = Config::load(path);
    ASSERT_NE(config, nullptr);
    
    // 不存在的键应返回默认值
    EXPECT_EQ(config->get<int>("nonexistent/key", 123), 123);
    EXPECT_FLOAT_EQ(config->get<double>("nonexistent/float", 3.14), 3.14);
    EXPECT_EQ(config->get<QString>("nonexistent/str", "default"), "default");
}

// ============================================================================
// 保存测试
// ============================================================================

TEST_F(ConfigTest, SaveAndReload) {
    QString path = tempDir->filePath("config.json");
    Config::createDefault(path);
    
    auto config = Config::load(path);
    ASSERT_NE(config, nullptr);
    
    config->set("test/value", 42);
    EXPECT_TRUE(config->save());
    
    // 重新加载
    auto config2 = Config::load(path);
    ASSERT_NE(config2, nullptr);
    EXPECT_EQ(config2->get<int>("test/value"), 42);
}

// ============================================================================
// 键操作测试
// ============================================================================

TEST_F(ConfigTest, Contains) {
    QString path = tempDir->filePath("config.json");
    Config::createDefault(path);
    
    auto config = Config::load(path);
    ASSERT_NE(config, nullptr);
    
    EXPECT_TRUE(config->contains("engine/maxTokens"));
    EXPECT_FALSE(config->contains("nonexistent/key"));
}

TEST_F(ConfigTest, Remove) {
    QString path = tempDir->filePath("config.json");
    Config::createDefault(path);
    
    auto config = Config::load(path);
    ASSERT_NE(config, nullptr);
    
    config->set("to/remove", 123);
    EXPECT_TRUE(config->contains("to/remove"));
    
    config->remove("to/remove");
    EXPECT_FALSE(config->contains("to/remove"));
}

TEST_F(ConfigTest, Keys) {
    QString path = tempDir->filePath("config.json");
    Config::createDefault(path);
    
    auto config = Config::load(path);
    ASSERT_NE(config, nullptr);
    
    QStringList rootKeys = config->keys();
    EXPECT_TRUE(rootKeys.contains("engine"));
    EXPECT_TRUE(rootKeys.contains("ui"));
    
    QStringList engineKeys = config->keys("engine");
    EXPECT_TRUE(engineKeys.contains("maxTokens"));
    EXPECT_TRUE(engineKeys.contains("temperature"));
}

// ============================================================================
// JSON 转换测试
// ============================================================================

TEST_F(ConfigTest, ToJson) {
    QString path = tempDir->filePath("config.json");
    Config::createDefault(path);
    
    auto config = Config::load(path);
    ASSERT_NE(config, nullptr);
    
    QJsonObject json = config->toJson();
    EXPECT_TRUE(json.contains("engine"));
    EXPECT_TRUE(json.contains("ui"));
}

TEST_F(ConfigTest, FromJson) {
    auto config = std::make_unique<Config>();
    
    QJsonObject json;
    json["key"] = "value";
    json["number"] = 42;
    config->fromJson(json);
    
    EXPECT_EQ(config->get<QString>("key"), "value");
    EXPECT_EQ(config->get<int>("number"), 42);
}
