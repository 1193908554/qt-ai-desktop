#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <memory>

namespace QtAIDesktop {

/**
 * @brief 模型信息结构
 */
struct ModelInfo {
    QString name;
    QString path;
    qint64 size;
    QString description;
};

/**
 * @brief 模型管理器
 * 
 * 负责模型的加载、卸载和管理
 */
class ModelManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(int modelCount READ modelCount NOTIFY modelCountChanged)

public:
    explicit ModelManager(QObject *parent = nullptr);
    ~ModelManager();

    /**
     * @brief 获取模型数量
     */
    int modelCount() const;

    /**
     * @brief 添加模型
     * @param name 模型名称
     * @param path 模型路径
     */
    Q_INVOKABLE void addModel(const QString &name, const QString &path);

    /**
     * @brief 移除模型
     * @param name 模型名称
     */
    Q_INVOKABLE void removeModel(const QString &name);

    /**
     * @brief 获取所有模型
     * @return 模型列表
     */
    Q_INVOKABLE QVariantList getModels() const;

    /**
     * @brief 获取模型信息
     * @param name 模型名称
     * @return 模型信息
     */
    Q_INVOKABLE QVariantMap getModelInfo(const QString &name) const;

signals:
    void modelCountChanged();
    void modelAdded(const QString &name);
    void modelRemoved(const QString &name);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace QtAIDesktop