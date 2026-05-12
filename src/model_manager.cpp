#include "qt-ai-desktop/model_manager.h"

#include <QMap>
#include <QMutex>

namespace QtAIDesktop {

class ModelManager::Impl {
public:
    QMap<QString, ModelInfo> models;
    QMutex mutex;
};

ModelManager::ModelManager(QObject *parent)
    : QObject(parent)
    , pImpl(std::make_unique<Impl>())
{
}

ModelManager::~ModelManager() = default;

int ModelManager::modelCount() const {
    QMutexLocker locker(&pImpl->mutex);
    return pImpl->models.size();
}

void ModelManager::addModel(const QString &name, const QString &path) {
    QMutexLocker locker(&pImpl->mutex);
    
    ModelInfo info;
    info.name = name;
    info.path = path;
    info.size = 0; // TODO: 获取文件大小
    info.description = QString("Model: %1").arg(name);
    
    pImpl->models[name] = info;
    
    emit modelCountChanged();
    emit modelAdded(name);
}

void ModelManager::removeModel(const QString &name) {
    QMutexLocker locker(&pImpl->mutex);
    
    if (pImpl->models.contains(name)) {
        pImpl->models.remove(name);
        emit modelCountChanged();
        emit modelRemoved(name);
    }
}

QVariantList ModelManager::getModels() const {
    QMutexLocker locker(&pImpl->mutex);
    
    QVariantList list;
    for (const auto &info : pImpl->models) {
        QVariantMap map;
        map["name"] = info.name;
        map["path"] = info.path;
        map["size"] = info.size;
        map["description"] = info.description;
        list.append(map);
    }
    
    return list;
}

QVariantMap ModelManager::getModelInfo(const QString &name) const {
    QMutexLocker locker(&pImpl->mutex);
    
    QVariantMap map;
    if (pImpl->models.contains(name)) {
        const auto &info = pImpl->models[name];
        map["name"] = info.name;
        map["path"] = info.path;
        map["size"] = info.size;
        map["description"] = info.description;
    }
    
    return map;
}

} // namespace QtAIDesktop