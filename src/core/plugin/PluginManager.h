#ifndef WRITESMART_PLUGIN_MANAGER_H
#define WRITESMART_PLUGIN_MANAGER_H

#include <QString>
#include <QList>
#include <memory>
#include "core/plugin/PluginInterface.h"

class AIProvider;

class PluginManager {
public:
    static PluginManager& instance();

    void scanPlugins(const QString& pluginDir);
    void loadPlugin(const QString& path);
    void unloadPlugin(const QString& pluginId);
    void unloadAll();

    QList<IPlugin*> loadedPlugins() const;
    QList<IPlugin*> pluginsOfType(PluginType type) const;

    std::shared_ptr<AIProvider> createProvider(const QString& pluginId);

private:
    PluginManager() = default;
    struct LoadedPlugin {
        IPlugin* instance = nullptr;
        void* library = nullptr;
    };
    QList<LoadedPlugin> m_plugins;
};

#endif // WRITESMART_PLUGIN_MANAGER_H
