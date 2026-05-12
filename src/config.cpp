#include "qt-ai-desktop/config.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

namespace QtAIDesktop {

// ============================================================================
// Config::Impl
// ============================================================================
class Config::Impl {
public:
    QString filePath;
    QJsonObject root;
};

// ============================================================================
// Config 实现
// ============================================================================

Config::Config(QObject *parent)
    : QObject(parent)
    , pImpl(std::make_unique<Impl>())
{
}

Config::~Config() = default;

std::shared_ptr<Config> Config::load(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open config file:" << filePath;
        return nullptr;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << error.errorString();
        return nullptr;
    }
    
    auto config = std::shared_ptr<Config>(new Config());
    config->pImpl->filePath = filePath;
    config->pImpl->root = doc.object();
    
    qDebug() << "Config loaded from:" << filePath;
    
    return config;
}

bool Config::createDefault(const QString &filePath) {
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    QJsonObject defaults;
    
    // 引擎配置
    QJsonObject engine;
    engine["maxTokens"] = 2048;
    engine["temperature"] = 0.7;
    engine["topP"] = 0.9;
    engine["topK"] = 40;
    engine["repeatPenalty"] = 1.1;
    engine["seed"] = -1;
    engine["backend"] = "auto";
    defaults["engine"] = engine;
    
    // 模型配置
    QJsonObject model;
    model["defaultPath"] = "";
    model["autoLoad"] = false;
    model["maxModels"] = 10;
    defaults["model"] = model;
    
    // UI 配置
    QJsonObject ui;
    ui["theme"] = "auto";
    ui["fontSize"] = 14;
    ui["windowWidth"] = 1200;
    ui["windowHeight"] = 800;
    ui["language"] = "auto";
    defaults["ui"] = ui;
    
    // 日志配置
    QJsonObject logging;
    logging["level"] = "info";
    logging["target"] = "console";
    logging["file"] = "logs/qt-ai-desktop.log";
    logging["maxFileSize"] = 10485760; // 10MB
    defaults["logging"] = logging;
    
    // 写入文件
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    file.write(QJsonDocument(defaults).toJson(QJsonDocument::Indented));
    return true;
}

void Config::set(const QString &key, const QVariant &value) {
    QStringList path = key.split('/');
    setInObject(pImpl->root, path, value);
    emit valueChanged(key, value);
}

bool Config::contains(const QString &key) const {
    return getVariant(key).isValid();
}

void Config::remove(const QString &key) {
    QStringList path = key.split('/');
    
    if (path.size() == 1) {
        pImpl->root.remove(path[0]);
    } else {
        QJsonObject obj = pImpl->root[path[0]].toObject();
        for (int i = 1; i < path.size() - 1; ++i) {
            obj = obj[path[i]].toObject();
        }
        obj.remove(path.last());
        
        QJsonObject parent;
        parent[path[0]] = pImpl->root[path[0]].toObject();
        // Rebuild properly
    }
    
    emit valueRemoved(key);
}

QStringList Config::keys() const {
    return pImpl->root.keys();
}

QStringList Config::keys(const QString &prefix) const {
    QStringList result;
    QStringList prefixParts = prefix.split('/');
    
    QJsonObject current = pImpl->root;
    for (const QString &part : prefixParts) {
        if (current.contains(part)) {
            current = current[part].toObject();
        } else {
            return result;
        }
    }
    
    for (auto it = current.begin(); it != current.end(); ++it) {
        result.append(it.key());
    }
    
    return result;
}

bool Config::save(const QString &filePath) {
    QString savePath = filePath.isEmpty() ? pImpl->filePath : filePath;
    
    if (savePath.isEmpty()) {
        qWarning() << "No file path specified for save";
        return false;
    }
    
    QFile file(savePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open file for writing:" << savePath;
        return false;
    }
    
    file.write(QJsonDocument(pImpl->root).toJson(QJsonDocument::Indented));
    
    pImpl->filePath = savePath;
    
    qDebug() << "Config saved to:" << savePath;
    
    return true;
}

bool Config::reload() {
    if (pImpl->filePath.isEmpty()) {
        return false;
    }
    
    QFile file(pImpl->filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        return false;
    }
    
    pImpl->root = doc.object();
    
    qDebug() << "Config reloaded from:" << pImpl->filePath;
    
    return true;
}

QJsonObject Config::toJson() const {
    return pImpl->root;
}

void Config::fromJson(const QJsonObject &obj) {
    pImpl->root = obj;
}

QString Config::filePath() const {
    return pImpl->filePath;
}

QVariant Config::getVariant(const QString &key) const {
    QStringList path = key.split('/');
    QJsonObject obj = pImpl->root;
    
    for (int i = 0; i < path.size() - 1; ++i) {
        if (!obj.contains(path[i]) || !obj[path[i]].isObject()) {
            return QVariant();
        }
        obj = obj[path[i]].toObject();
    }
    
    QJsonValue val = obj[path.last()];
    if (val.isUndefined() || val.isNull()) {
        return QVariant();
    }
    
    return val.toVariant();
}

void Config::setInObject(QJsonObject &obj, const QStringList &path, const QVariant &value) {
    if (path.size() == 1) {
        obj[path[0]] = QJsonValue::fromVariant(value);
        return;
    }
    
    QJsonObject child = obj[path[0]].toObject();
    setInObject(child, path.mid(1), value);
    obj[path[0]] = child;
}

} // namespace QtAIDesktop
