#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <QUrl>
#include <QDateTime>
#include <memory>

namespace QtAIDesktop {

/**
 * @brief 模型类型枚举
 */
enum class ModelType {
    LLM,        ///< 大语言模型
    Embedding,  ///< 嵌入模型
    Image,      ///< 图像生成模型
    Audio,      ///< 语音模型（TTS/ASR）
    Multimodal  ///< 多模态模型
};

/**
 * @brief 量化类型
 */
enum class QuantizationType {
    None,       ///< 未量化
    Q4_0,       ///< 4-bit 量化
    Q4_1,       ///< 4-bit 量化 (改进)
    Q4_K_M,     ///< 4-bit K-quant 中等
    Q4_K_S,     ///< 4-bit K-quant 小
    Q5_0,       ///< 5-bit 量化
    Q5_1,       ///< 5-bit 量化 (改进)
    Q5_K_M,     ///< 5-bit K-quant 中等
    Q5_K_S,     ///< 5-bit K-quant 小
    Q6_K,       ///< 6-bit K-quant
    Q8_0,       ///< 8-bit 量化
    F16,        ///< 16-bit 浮点
    F32         ///< 32-bit 浮点
};

/**
 * @brief 模型信息结构
 */
struct ModelInfo {
    QString id;                     ///< 唯一标识符
    QString name;                   ///< 显示名称
    QString path;                   ///< 本地路径
    QUrl downloadUrl;               ///< 下载地址
    ModelType type = ModelType::LLM;///< 模型类型
    QuantizationType quant = QuantizationType::None; ///< 量化类型
    qint64 size = 0;                ///< 文件大小（字节）
    qint64 memoryRequired = 0;      ///< 所需内存（字节）
    QString family;                 ///< 模型系列（如 Llama, Mistral）
    int parameterCount = 0;         ///< 参数量（亿）
    int contextLength = 2048;       ///< 上下文长度
    QString description;            ///< 描述
    QDateTime lastUsed;             ///< 最后使用时间
    bool isFavorite = false;        ///< 是否收藏
    bool isInstalled = false;       ///< 是否已下载
    
    // 序列化方法
    QVariantMap toMap() const;
    static ModelInfo fromMap(const QVariantMap &map);
};

/**
 * @brief 模型管理器
 * 
 * 负责模型的发现、下载、加载、卸载和管理。
 * 支持本地模型和远程模型仓库。
 * 
 * @example
 * @code
 * auto manager = new ModelManager(this);
 * 
 * // 扫描本地模型
 * manager->scanLocalModels("/path/to/models");
 * 
 * // 添加模型
 * manager->addModel("Llama-3-8B", "/path/to/llama-8b.gguf");
 * 
 * // 获取模型列表
 * auto models = manager->getModels();
 * @endcode
 */
class ModelManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(int modelCount READ modelCount NOTIFY modelCountChanged)
    Q_PROPERTY(int installedCount READ installedCount NOTIFY modelCountChanged)
    Q_PROPERTY(QString modelsPath READ modelsPath WRITE setModelsPath NOTIFY modelsPathChanged)

public:
    explicit ModelManager(QObject *parent = nullptr);
    ~ModelManager();

    // 属性访问器
    int modelCount() const;
    int installedCount() const;
    QString modelsPath() const;
    void setModelsPath(const QString &path);

    /**
     * @brief 扫描本地模型目录
     * @param path 目录路径，为空则使用默认路径
     * @return 发现的模型数量
     */
    Q_INVOKABLE int scanLocalModels(const QString &path = QString());

    /**
     * @brief 添加模型（从本地路径）
     * @param name 模型名称
     * @param path 模型文件路径
     * @return 模型ID
     */
    Q_INVOKABLE QString addModel(const QString &name, const QString &path);

    /**
     * @brief 注册远程模型
     * @param info 模型信息（需要包含downloadUrl）
     * @return 模型ID
     */
    Q_INVOKABLE QString registerModel(const ModelInfo &info);

    /**
     * @brief 移除模型
     * @param modelId 模型ID
     * @param deleteFile 是否同时删除文件
     * @return 是否成功
     */
    Q_INVOKABLE bool removeModel(const QString &modelId, bool deleteFile = false);

    /**
     * @brief 获取所有模型
     * @param filterType 可选：按类型过滤
     * @return 模型列表
     */
    Q_INVOKABLE QVariantList getModels(int filterType = -1) const;

    /**
     * @brief 获取单个模型信息
     * @param modelId 模型ID
     * @return 模型信息
     */
    Q_INVOKABLE QVariantMap getModelInfo(const QString &modelId) const;

    /**
     * @brief 搜索模型
     * @param query 搜索关键词
     * @return 匹配的模型列表
     */
    Q_INVOKABLE QVariantList searchModels(const QString &query) const;

    /**
     * @brief 获取推荐模型列表
     * @return 推荐的模型
     */
    Q_INVOKABLE QVariantList getRecommendedModels() const;

    /**
     * @brief 更新模型信息
     * @param modelId 模型ID
     * @param info 新的模型信息
     */
    Q_INVOKABLE void updateModelInfo(const QString &modelId, const QVariantMap &info);

    /**
     * @brief 设置/取消收藏
     * @param modelId 模型ID
     * @param favorite 是否收藏
     */
    Q_INVOKABLE void setFavorite(const QString &modelId, bool favorite);

    /**
     * @brief 获取收藏的模型
     */
    Q_INVOKABLE QVariantList getFavoriteModels() const;

    /**
     * @brief 导出模型列表
     * @param filePath 导出文件路径
     */
    Q_INVOKABLE bool exportModelList(const QString &filePath) const;

    /**
     * @brief 导入模型列表
     * @param filePath 导入文件路径
     */
    Q_INVOKABLE int importModelList(const QString &filePath);

signals:
    /**
     * @brief 模型数量变化
     */
    void modelCountChanged();

    /**
     * @brief 模型路径变化
     */
    void modelsPathChanged();

    /**
     * @brief 新模型添加
     * @param modelId 模型ID
     */
    void modelAdded(const QString &modelId);

    /**
     * @brief 模型移除
     * @param modelId 模型ID
     */
    void modelRemoved(const QString &modelId);

    /**
     * @brief 模型下载进度
     * @param modelId 模型ID
     * @param bytesReceived 已接收字节
     * @param bytesTotal 总字节
     */
    void downloadProgress(const QString &modelId, qint64 bytesReceived, qint64 bytesTotal);

    /**
     * @brief 模型下载完成
     * @param modelId 模型ID
     * @param success 是否成功
     * @param filePath 下载的文件路径
     */
    void downloadComplete(const QString &modelId, bool success, const QString &filePath);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace QtAIDesktop
