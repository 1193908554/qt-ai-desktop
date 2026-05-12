#include "qt-ai-desktop/model_manager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMutex>
#include <QUuid>
#include <QStandardPaths>
#include <QDebug>

namespace QtAIDesktop {

// ============================================================================
// ModelInfo 序列化
// ============================================================================

QVariantMap ModelInfo::toMap() const {
    QVariantMap map;
    map["id"] = id;
    map["name"] = name;
    map["path"] = path;
    map["downloadUrl"] = downloadUrl.toString();
    map["type"] = static_cast<int>(type);
    map["quant"] = static_cast<int>(quant);
    map["size"] = size;
    map["memoryRequired"] = memoryRequired;
    map["family"] = family;
    map["parameterCount"] = parameterCount;
    map["contextLength"] = contextLength;
    map["description"] = description;
    map["lastUsed"] = lastUsed.toString(Qt::ISODate);
    map["isFavorite"] = isFavorite;
    map["isInstalled"] = isInstalled;
    return map;
}

ModelInfo ModelInfo::fromMap(const QVariantMap &map) {
    ModelInfo info;
    info.id = map["id"].toString();
    info.name = map["name"].toString();
    info.path = map["path"].toString();
    info.downloadUrl = QUrl(map["downloadUrl"].toString());
    info.type = static_cast<ModelType>(map["type"].toInt());
    info.quant = static_cast<QuantizationType>(map["quant"].toInt());
    info.size = map["size"].toLongLong();
    info.memoryRequired = map["memoryRequired"].toLongLong();
    info.family = map["family"].toString();
    info.parameterCount = map["parameterCount"].toInt();
    info.contextLength = map["contextLength"].toInt();
    info.description = map["description"].toString();
    info.lastUsed = QDateTime::fromString(map["lastUsed"].toString(), Qt::ISODate);
    info.isFavorite = map["isFavorite"].toBool();
    info.isInstalled = map["isInstalled"].toBool();
    return info;
}

// ============================================================================
// ModelManager::Impl - 私有实现
// ============================================================================
class ModelManager::Impl {
public:
    // 模型存储
    QMap<QString, ModelInfo> models;
    QString modelsPath;
    
    // 支持的模型文件扩展名
    QStringList supportedExtensions = {
        ".gguf", ".ggml", ".bin", ".onnx", 
        ".pt", ".pth", ".safetensors", ".bin"
    };
    
    // 方法
    QString generateId() const {
        return QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);
    }
    
    QString defaultModelsPath() const {
        return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) 
               + "/models";
    }
    
    QString ensureModelsPath() {
        if (modelsPath.isEmpty()) {
            modelsPath = defaultModelsPath();
        }
        
        QDir dir(modelsPath);
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        
        return modelsPath;
    }
    
    // 解析模型文件名信息
    void parseModelFileInfo(ModelInfo &info, const QFileInfo &fileInfo) {
        info.path = fileInfo.absoluteFilePath();
        info.size = fileInfo.size();
        info.isInstalled = true;
        
        QString baseName = fileInfo.completeBaseName().toLower();
        
        // 尝试解析参数量
        if (baseName.contains("7b") || baseName.contains("70b")) {
            info.parameterCount = baseName.contains("70b") ? 70 : 7;
        } else if (baseName.contains("13b")) {
            info.parameterCount = 13;
        } else if (baseName.contains("8b")) {
            info.parameterCount = 8;
        } else if (baseName.contains("3b")) {
            info.parameterCount = 3;
        } else if (baseName.contains("1b")) {
            info.parameterCount = 1;
        }
        
        // 尝试解析量化类型
        if (baseName.contains("q4_0")) {
            info.quant = QuantizationType::Q4_0;
        } else if (baseName.contains("q4_k_m")) {
            info.quant = QuantizationType::Q4_K_M;
        } else if (baseName.contains("q5_k_m")) {
            info.quant = QuantizationType::Q5_K_M;
        } else if (baseName.contains("q8_0")) {
            info.quant = QuantizationType::Q8_0;
        } else if (baseName.contains("f16")) {
            info.quant = QuantizationType::F16;
        }
        
        // 估算内存需求（基于大小 * 1.2）
        info.memoryRequired = static_cast<qint64>(info.size * 1.2);
    }
    
    // 保存模型列表到文件
    bool saveToFile() {
        QJsonArray arr;
        for (auto it = models.begin(); it != models.end(); ++it) {
            arr.append(QJsonObject::fromVariantMap(it.value().toMap()));
        }
        
        QFile file(ensureModelsPath() + "/models.json");
        if (!file.open(QIODevice::WriteOnly)) {
            qWarning() << "Failed to save models list:" << file.errorString();
            return false;
        }
        
        file.write(QJsonDocument(arr).toJson());
        return true;
    }
    
    // 从文件加载模型列表
    bool loadFromFile() {
        QFile file(ensureModelsPath() + "/models.json");
        if (!file.exists()) {
            return true;  // 文件不存在是正常的
        }
        
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "Failed to load models list:" << file.errorString();
            return false;
        }
        
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (!doc.isArray()) {
            return false;
        }
        
        QJsonArray arr = doc.array();
        models.clear();
        
        for (const auto &val : arr) {
            ModelInfo info = ModelInfo::fromMap(val.toObject().toVariantMap());
            if (!info.id.isEmpty()) {
                models[info.id] = info;
            }
        }
        
        return true;
    }
};

// ============================================================================
// ModelManager 实现
// ============================================================================

ModelManager::ModelManager(QObject *parent)
    : QObject(parent)
    , pImpl(std::make_unique<Impl>())
{
    // 加载已保存的模型列表
    pImpl->loadFromFile();
    
    qDebug() << "ModelManager created, loaded" << models.size() << "models";
}

ModelManager::~ModelManager() = default;

// ----------------------------------------------------------------------------
// 属性访问器
// ----------------------------------------------------------------------------

int ModelManager::modelCount() const {
    return pImpl->models.size();
}

int ModelManager::installedCount() const {
    int count = 0;
    for (const auto &info : pImpl->models) {
        if (info.isInstalled) {
            count++;
        }
    }
    return count;
}

QString ModelManager::modelsPath() const {
    return pImpl->modelsPath;
}

void ModelManager::setModelsPath(const QString &path) {
    if (pImpl->modelsPath != path) {
        pImpl->modelsPath = path;
        emit modelsPathChanged();
        
        // 重新扫描
        scanLocalModels(path);
    }
}

// ----------------------------------------------------------------------------
// 模型管理
// ----------------------------------------------------------------------------

int ModelManager::scanLocalModels(const QString &path) {
    QString scanPath = path.isEmpty() ? pImpl->ensureModelsPath() : path;
    
    QDir dir(scanPath);
    if (!dir.exists()) {
        qWarning() << "Models directory does not exist:" << scanPath;
        return 0;
    }
    
    qDebug() << "Scanning models in:" << scanPath;
    
    int found = 0;
    
    // 扫描所有支持的模型文件
    for (const QString &ext : pImpl->supportedExtensions) {
        QStringList files = dir.entryList(QStringList() << ("*" + ext), QDir::Files);
        
        for (const QString &fileName : files) {
            QFileInfo fileInfo(dir, fileName);
            
            // 检查是否已存在
            bool exists = false;
            for (const auto &info : pImpl->models) {
                if (info.path == fileInfo.absoluteFilePath()) {
                    exists = true;
                    break;
                }
            }
            
            if (!exists) {
                ModelInfo info;
                info.id = pImpl->generateId();
                info.name = fileInfo.completeBaseName();
                pImpl->parseModelFileInfo(info, fileInfo);
                
                pImpl->models[info.id] = info;
                found++;
                
                qDebug() << "Found model:" << info.name 
                         << "(" << (info.size / 1024 / 1024) << "MB)";
            }
        }
    }
    
    if (found > 0) {
        pImpl->saveToFile();
        emit modelCountChanged();
    }
    
    qDebug() << "Scan complete. Found" << found << "new models";
    
    return found;
}

QString ModelManager::addModel(const QString &name, const QString &path) {
    QFileInfo fileInfo(path);
    
    if (!fileInfo.exists()) {
        qWarning() << "Model file does not exist:" << path;
        return QString();
    }
    
    // 检查是否已存在
    for (const auto &info : pImpl->models) {
        if (info.path == fileInfo.absoluteFilePath()) {
            qDebug() << "Model already registered:" << info.id;
            return info.id;
        }
    }
    
    ModelInfo info;
    info.id = pImpl->generateId();
    info.name = name.isEmpty() ? fileInfo.completeBaseName() : name;
    pImpl->parseModelFileInfo(info, fileInfo);
    
    pImpl->models[info.id] = info;
    pImpl->saveToFile();
    
    emit modelCountChanged();
    emit modelAdded(info.id);
    
    qDebug() << "Model added:" << info.name << "(" << info.id << ")";
    
    return info.id;
}

QString ModelManager::registerModel(const ModelInfo &info) {
    // 检查是否已存在
    for (const auto &existing : pImpl->models) {
        if (existing.downloadUrl == info.downloadUrl) {
            qDebug() << "Model already registered:" << existing.id;
            return existing.id;
        }
    }
    
    ModelInfo newInfo = info;
    if (newInfo.id.isEmpty()) {
        newInfo.id = pImpl->generateId();
    }
    
    pImpl->models[newInfo.id] = newInfo;
    pImpl->saveToFile();
    
    emit modelCountChanged();
    emit modelAdded(newInfo.id);
    
    qDebug() << "Remote model registered:" << newInfo.name << "(" << newInfo.id << ")";
    
    return newInfo.id;
}

bool ModelManager::removeModel(const QString &modelId, bool deleteFile) {
    if (!pImpl->models.contains(modelId)) {
        qWarning() << "Model not found:" << modelId;
        return false;
    }
    
    ModelInfo info = pImpl->models[modelId];
    
    // 删除文件
    if (deleteFile && !info.path.isEmpty()) {
        QFile file(info.path);
        if (file.exists()) {
            if (!file.remove()) {
                qWarning() << "Failed to delete model file:" << info.path;
                return false;
            }
            qDebug() << "Model file deleted:" << info.path;
        }
    }
    
    pImpl->models.remove(modelId);
    pImpl->saveToFile();
    
    emit modelCountChanged();
    emit modelRemoved(modelId);
    
    qDebug() << "Model removed:" << info.name << "(" << modelId << ")";
    
    return true;
}

QVariantList ModelManager::getModels(int filterType) const {
    QVariantList result;
    
    for (auto it = pImpl->models.begin(); it != pImpl->models.end(); ++it) {
        if (filterType < 0 || static_cast<int>(it.value().type) == filterType) {
            result.append(it.value().toMap());
        }
    }
    
    return result;
}

QVariantMap ModelManager::getModelInfo(const QString &modelId) const {
    if (!pImpl->models.contains(modelId)) {
        qWarning() << "Model not found:" << modelId;
        return QVariantMap();
    }
    
    return pImpl->models[modelId].toMap();
}

QVariantList ModelManager::searchModels(const QString &query) const {
    QVariantList result;
    QString lowerQuery = query.toLower();
    
    for (auto it = pImpl->models.begin(); it != pImpl->models.end(); ++it) {
        const ModelInfo &info = it.value();
        
        if (info.name.toLower().contains(lowerQuery) ||
            info.family.toLower().contains(lowerQuery) ||
            info.description.toLower().contains(lowerQuery)) {
            result.append(info.toMap());
        }
    }
    
    return result;
}

QVariantList ModelManager::getRecommendedModels() const {
    // 返回按使用频率和评分排序的模型
    QVariantList result;
    
    QList<ModelInfo> sorted;
    for (auto it = pImpl->models.begin(); it != pImpl->models.end(); ++it) {
        sorted.append(it.value());
    }
    
    // 排序：收藏优先，然后按最后使用时间
    std::sort(sorted.begin(), sorted.end(), 
              [](const ModelInfo &a, const ModelInfo &b) {
                  if (a.isFavorite != b.isFavorite) {
                      return a.isFavorite > b.isFavorite;
                  }
                  return a.lastUsed > b.lastUsed;
              });
    
    for (const auto &info : sorted) {
        result.append(info.toMap());
    }
    
    return result;
}

void ModelManager::updateModelInfo(const QString &modelId, const QVariantMap &info) {
    if (!pImpl->models.contains(modelId)) {
        qWarning() << "Model not found:" << modelId;
        return;
    }
    
    ModelInfo &existing = pImpl->models[modelId];
    
    if (info.contains("name")) {
        existing.name = info["name"].toString();
    }
    if (info.contains("description")) {
        existing.description = info["description"].toString();
    }
    if (info.contains("contextLength")) {
        existing.contextLength = info["contextLength"].toInt();
    }
    
    pImpl->saveToFile();
}

void ModelManager::setFavorite(const QString &modelId, bool favorite) {
    if (!pImpl->models.contains(modelId)) {
        qWarning() << "Model not found:" << modelId;
        return;
    }
    
    pImpl->models[modelId].isFavorite = favorite;
    pImpl->saveToFile();
}

QVariantList ModelManager::getFavoriteModels() const {
    QVariantList result;
    
    for (auto it = pImpl->models.begin(); it != pImpl->models.end(); ++it) {
        if (it.value().isFavorite) {
            result.append(it.value().toMap());
        }
    }
    
    return result;
}

bool ModelManager::exportModelList(const QString &filePath) const {
    QJsonArray arr;
    for (auto it = pImpl->models.begin(); it != pImpl->models.end(); ++it) {
        arr.append(QJsonObject::fromVariantMap(it.value().toMap()));
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to export:" << file.errorString();
        return false;
    }
    
    file.write(QJsonDocument(arr).toJson());
    qDebug() << "Model list exported to:" << filePath;
    
    return true;
}

int ModelManager::importModelList(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to import:" << file.errorString();
        return 0;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray()) {
        return 0;
    }
    
    QJsonArray arr = doc.array();
    int imported = 0;
    
    for (const auto &val : arr) {
        ModelInfo info = ModelInfo::fromMap(val.toObject().toVariantMap());
        
        // 检查是否已存在
        bool exists = false;
        for (const auto &existing : pImpl->models) {
            if (existing.path == info.path || 
                (!info.downloadUrl.isEmpty() && existing.downloadUrl == info.downloadUrl)) {
                exists = true;
                break;
            }
        }
        
        if (!exists) {
            pImpl->models[info.id] = info;
            imported++;
        }
    }
    
    if (imported > 0) {
        pImpl->saveToFile();
        emit modelCountChanged();
    }
    
    qDebug() << "Imported" << imported << "models";
    
    return imported;
}

} // namespace QtAIDesktop
