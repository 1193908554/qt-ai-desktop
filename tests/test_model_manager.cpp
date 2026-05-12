#include <gtest/gtest.h>
#include "qt-ai-desktop/model_manager.h"
#include <QTemporaryDir>
#include <QFile>

using namespace QtAIDesktop;

class ModelManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = std::make_unique<ModelManager>();
    }
    
    void TearDown() override {
        manager.reset();
    }
    
    std::unique_ptr<ModelManager> manager;
};

// ============================================================================
// 基本测试
// ============================================================================

TEST_F(ModelManagerTest, InitialState) {
    EXPECT_EQ(manager->modelCount(), 0);
}

// ============================================================================
// 添加模型测试
// ============================================================================

TEST_F(ModelManagerTest, AddModel) {
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    
    // 创建临时模型文件
    QString modelPath = tempDir.filePath("test-model.gguf");
    QFile file(modelPath);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    file.write(QByteArray(1024, 'x'));  // 写入 1KB 数据
    file.close();
    
    QString id = manager->addModel("Test Model", modelPath);
    EXPECT_FALSE(id.isEmpty());
    EXPECT_EQ(manager->modelCount(), 1);
    
    // 验证模型信息
    QVariantMap info = manager->getModelInfo(id);
    EXPECT_EQ(info["name"].toString(), "Test Model");
    EXPECT_TRUE(info["isInstalled"].toBool());
}

TEST_F(ModelManagerTest, AddModelDuplicate) {
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    
    QString modelPath = tempDir.filePath("test.gguf");
    QFile file(modelPath);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    file.write("test");
    file.close();
    
    QString id1 = manager->addModel("Model 1", modelPath);
    QString id2 = manager->addModel("Model 1", modelPath);
    
    // 同一路径不应重复添加
    EXPECT_EQ(manager->modelCount(), 1);
}

// ============================================================================
// 移除模型测试
// ============================================================================

TEST_F(ModelManagerTest, RemoveModel) {
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    
    QString modelPath = tempDir.filePath("test.gguf");
    QFile file(modelPath);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    file.write("test");
    file.close();
    
    QString id = manager->addModel("Test Model", modelPath);
    EXPECT_EQ(manager->modelCount(), 1);
    
    bool removed = manager->removeModel(id);
    EXPECT_TRUE(removed);
    EXPECT_EQ(manager->modelCount(), 0);
}

TEST_F(ModelManagerTest, RemoveNonexistent) {
    bool removed = manager->removeModel("nonexistent-id");
    EXPECT_FALSE(removed);
}

// ============================================================================
// 搜索测试
// ============================================================================

TEST_F(ModelManagerTest, SearchModels) {
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    
    // 创建多个模型
    for (const QString &name : {"llama-7b", "mistral-7b", "phi-3b"}) {
        QString path = tempDir.filePath(name + ".gguf");
        QFile file(path);
        file.open(QIODevice::WriteOnly);
        file.write("test");
        file.close();
        manager->addModel(name, path);
    }
    
    // 搜索
    auto results = manager->searchModels("llama");
    EXPECT_EQ(results.size(), 1);
    
    results = manager->searchModels("7b");
    EXPECT_EQ(results.size(), 2);  // llama-7b 和 mistral-7b
    
    results = manager->searchModels("nonexistent");
    EXPECT_EQ(results.size(), 0);
}

// ============================================================================
// 收藏测试
// ============================================================================

TEST_F(ModelManagerTest, FavoriteModels) {
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    
    QString modelPath = tempDir.filePath("test.gguf");
    QFile file(modelPath);
    file.open(QIODevice::WriteOnly);
    file.write("test");
    file.close();
    
    QString id = manager->addModel("Test", modelPath);
    
    // 设置收藏
    manager->setFavorite(id, true);
    auto favorites = manager->getFavoriteModels();
    EXPECT_EQ(favorites.size(), 1);
    
    // 取消收藏
    manager->setFavorite(id, false);
    favorites = manager->getFavoriteModels();
    EXPECT_EQ(favorites.size(), 0);
}

// ============================================================================
// 获取模型列表测试
// ============================================================================

TEST_F(ModelManagerTest, GetModels) {
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    
    // 初始为空
    auto models = manager->getModels();
    EXPECT_EQ(models.size(), 0);
    
    // 添加模型
    QString path = tempDir.filePath("model.gguf");
    QFile file(path);
    file.open(QIODevice::WriteOnly);
    file.write("test");
    file.close();
    
    manager->addModel("Model", path);
    models = manager->getModels();
    EXPECT_EQ(models.size(), 1);
}

// ============================================================================
// 导入导出测试
// ============================================================================

TEST_F(ModelManagerTest, ExportImport) {
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    
    // 添加模型
    QString path = tempDir.filePath("model.gguf");
    QFile file(path);
    file.open(QIODevice::WriteOnly);
    file.write("test");
    file.close();
    
    manager->addModel("Export Test", path);
    
    // 导出
    QString exportPath = tempDir.filePath("export.json");
    EXPECT_TRUE(manager->exportModelList(exportPath));
    
    // 创建新的管理器并导入
    ModelManager newManager;
    int imported = newManager.importModelList(exportPath);
    EXPECT_GE(imported, 1);
}
