#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QVariant>
#include <QJsonObject>
#include <memory>

namespace QtAIDesktop {

/**
 * @brief 配置管理器
 * 
 * JSON 格式的配置文件管理，支持默认值、热更新、观察者模式。
 * 
 * @example
 * @code
 * auto config = Config::load("config.json");
 * 
 * // 读取配置
 * int maxTokens = config->get("engine/maxTokens", 2048);
 * float temp = config->get("engine/temperature", 0.7f);
 * 
 * // 写入配置
 * config->set("engine/temperature", 0.9f);
 * config->save();
 * 
 * // 监听变化
 * connect(config, &Config::valueChanged, [](const QString &key, const QVariant &val) {
 *     qDebug() << "Config changed:" << key << "=" << val;
 * });
 * @endcode
 */
class Config : public QObject {
    Q_OBJECT

public:
    explicit Config(QObject *parent = nullptr);
    ~Config();

    /**
     * @brief 从文件加载配置
     * @param filePath 配置文件路径
     * @return 配置实例（失败返回 nullptr）
     */
    static std::shared_ptr<Config> load(const QString &filePath);

    /**
     * @brief 创建默认配置文件
     * @param filePath 配置文件路径
     * @return 是否成功
     */
    static bool createDefault(const QString &filePath);

    /**
     * @brief 获取配置值
     * @param key 配置键（支持 "/" 分隔的路径）
     * @param defaultValue 默认值
     */
    template<typename T>
    T get(const QString &key, const T &defaultValue = T()) const {
        QVariant val = getVariant(key);
        if (val.isValid() && val.canConvert<T>()) {
            return val.value<T>();
        }
        return defaultValue;
    }

    /**
     * @brief 设置配置值
     * @param key 配置键
     * @param value 值
     */
    void set(const QString &key, const QVariant &value);

    /**
     * @brief 检查键是否存在
     */
    bool contains(const QString &key) const;

    /**
     * @brief 删除键
     */
    void remove(const QString &key);

    /**
     * @brief 获取所有键
     */
    QStringList keys() const;

    /**
     * @brief 获取指定前缀下的所有键
     */
    QStringList keys(const QString &prefix) const;

    /**
     * @brief 保存到文件
     */
    bool save(const QString &filePath = QString());

    /**
     * @brief 重新加载文件
     */
    bool reload();

    /**
     * @brief 获取原始 JSON 对象
     */
    QJsonObject toJson() const;

    /**
     * @brief 从 JSON 对象加载
     */
    void fromJson(const QJsonObject &obj);

    /**
     * @brief 获取配置文件路径
     */
    QString filePath() const;

signals:
    /**
     * @brief 配置值变化
     */
    void valueChanged(const QString &key, const QVariant &newValue);
    void valueRemoved(const QString &key);

private:
    QVariant getVariant(const QString &key) const;
    QJsonObject getObject(const QStringList &path) const;
    void setInObject(QJsonObject &obj, const QStringList &path, const QVariant &value);

    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace QtAIDesktop
