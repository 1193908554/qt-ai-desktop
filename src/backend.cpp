#include "qt-ai-desktop/backend.h"
#include <QMap>
#include <QMutex>
#include <QDebug>

namespace QtAIDesktop {

// ============================================================================
// 后端注册表
// ============================================================================
struct BackendFactory::Registry {
    QMap<QString, std::function<std::unique_ptr<IBackend>()>> creators;
    QMutex mutex;
};

BackendFactory::Registry& BackendFactory::registry() {
    static Registry instance;
    return instance;
}

void BackendFactory::registerBackend(const QString &name,
                                      std::function<std::unique_ptr<IBackend>()> creator) {
    QMutexLocker locker(&registry().mutex);
    registry().creators[name] = std::move(creator);
    qDebug() << "Backend registered:" << name;
}

std::unique_ptr<IBackend> BackendFactory::create(const QString &name) {
    QMutexLocker locker(&registry().mutex);
    
    auto it = registry().creators.find(name);
    if (it != registry().creators.end()) {
        return it.value();
    }
    
    qWarning() << "Backend not found:" << name;
    return nullptr;
}

QStringList BackendFactory::availableBackends() {
    QMutexLocker locker(&registry().mutex);
    return registry().creators.keys();
}

QVariantMap BackendFactory::backendCapabilities() {
    QMutexLocker locker(&registry().mutex);
    QVariantMap result;
    
    for (auto it = registry().creators.begin(); it != registry().creators.end(); ++it) {
        auto backend = it.value();
        auto instance = backend();
        QVariantMap caps;
        BackendCapabilities c = instance->capabilities();
        caps["name"] = instance->name();
        caps["version"] = instance->version();
        caps["description"] = instance->description();
        caps["chat"] = c.supportsChat;
        caps["embeddings"] = c.supportsEmbeddings;
        caps["image"] = c.supportsImageGeneration;
        caps["speech"] = c.supportsSpeechRecognition;
        caps["streaming"] = c.supportsStreaming;
        caps["gpu"] = c.supportsGPU;
        caps["formats"] = c.supportedFormats;
        result[it.key()] = caps;
    }
    
    return result;
}

} // namespace QtAIDesktop
