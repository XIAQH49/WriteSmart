#include "core/plugin/PluginManager.h"
#include "utils/Logger.h"
#include <QLibrary>
#include <QDir>

PluginManager& PluginManager::instance()
{
    static PluginManager mgr;
    return mgr;
}

void PluginManager::scanPlugins(const QString& pluginDir)
{
    QDir dir(pluginDir);
    if (!dir.exists()) return;

    const auto files = dir.entryInfoList({"*.dll", "*.so"}, QDir::Files);
    for (const auto& info : files) {
        loadPlugin(info.absoluteFilePath());
    }
}

void PluginManager::loadPlugin(const QString& path)
{
    QLibrary* lib = new QLibrary(path);
    if (!lib->load()) {
        Logger::warning("Failed to load plugin: " + path + " - " + lib->errorString());
        delete lib;
        return;
    }

    auto createFunc = reinterpret_cast<CreatePluginFunc>(lib->resolve("createPlugin"));
    if (!createFunc) {
        Logger::warning("Plugin missing createPlugin symbol: " + path);
        lib->unload();
        delete lib;
        return;
    }

    IPlugin* plugin = createFunc();
    if (!plugin) {
        Logger::warning("Plugin createPlugin returned null: " + path);
        lib->unload();
        delete lib;
        return;
    }

    if (plugin->initialize()) {
        LoadedPlugin lp;
        lp.instance = plugin;
        lp.library = lib;
        m_plugins.append(lp);
        Logger::info("Plugin loaded: " + plugin->name() + " v" + plugin->version());
    } else {
        Logger::warning("Plugin failed to initialize: " + plugin->name());
        auto destroyFunc = reinterpret_cast<DestroyPluginFunc>(lib->resolve("destroyPlugin"));
        if (destroyFunc) destroyFunc(plugin);
        lib->unload();
        delete lib;
    }
}

void PluginManager::unloadPlugin(const QString& pluginId)
{
    for (int i = 0; i < m_plugins.size(); ++i) {
        if (m_plugins[i].instance->id() == pluginId) {
            m_plugins[i].instance->shutdown();
            auto* lib = static_cast<QLibrary*>(m_plugins[i].library);
            auto destroyFunc = reinterpret_cast<DestroyPluginFunc>(lib->resolve("destroyPlugin"));
            if (destroyFunc) destroyFunc(m_plugins[i].instance);
            lib->unload();
            delete lib;
            m_plugins.removeAt(i);
            return;
        }
    }
}

void PluginManager::unloadAll()
{
    for (auto& lp : m_plugins) {
        lp.instance->shutdown();
        auto* lib = static_cast<QLibrary*>(lp.library);
        auto destroyFunc = reinterpret_cast<DestroyPluginFunc>(lib->resolve("destroyPlugin"));
        if (destroyFunc) destroyFunc(lp.instance);
        lib->unload();
        delete lib;
    }
    m_plugins.clear();
}

QList<IPlugin*> PluginManager::loadedPlugins() const
{
    QList<IPlugin*> list;
    for (const auto& lp : m_plugins) {
        list.append(lp.instance);
    }
    return list;
}

QList<IPlugin*> PluginManager::pluginsOfType(PluginType type) const
{
    QList<IPlugin*> list;
    for (const auto& lp : m_plugins) {
        if (type == PluginType::AIProvider &&
            dynamic_cast<IAIProviderPlugin*>(lp.instance)) {
            list.append(lp.instance);
        }
    }
    return list;
}

std::shared_ptr<AIProvider> PluginManager::createProvider(const QString& pluginId)
{
    for (const auto& lp : m_plugins) {
        if (lp.instance->id() == pluginId) {
            auto* aiPlugin = dynamic_cast<IAIProviderPlugin*>(lp.instance);
            if (aiPlugin) {
                void* raw = aiPlugin->createProvider();
                if (raw) {
                    return std::shared_ptr<AIProvider>(static_cast<AIProvider*>(raw),
                        [aiPlugin](AIProvider* p) { aiPlugin->destroyProvider(p); });
                }
            }
        }
    }
    return nullptr;
}
