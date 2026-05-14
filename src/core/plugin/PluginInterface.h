#ifndef WRITESMART_PLUGIN_INTERFACE_H
#define WRITESMART_PLUGIN_INTERFACE_H

#include <QString>
#include <memory>

class IPlugin {
public:
    virtual ~IPlugin() = default;

    virtual QString id() const = 0;
    virtual QString name() const = 0;
    virtual QString version() const = 0;
    virtual QString description() const = 0;

    virtual bool initialize() = 0;
    virtual void shutdown() = 0;

    virtual bool isInitialized() const = 0;
};

enum class PluginType {
    AIProvider,
    ExportFormat,
    Tool,
    Unknown
};

class IAIProviderPlugin : public IPlugin {
public:
    virtual void* createProvider() = 0;     // 返回 AIProvider*, 避免头文件依赖
    virtual void destroyProvider(void* provider) = 0;
};

using PluginPtr = std::unique_ptr<IPlugin>;

extern "C" {
    using CreatePluginFunc = IPlugin* (*)();
    using DestroyPluginFunc = void (*)(IPlugin*);
}

#define PLUGIN_EXPORT_CREATE extern "C" __declspec(dllexport) IPlugin* createPlugin()
#define PLUGIN_EXPORT_DESTROY extern "C" __declspec(dllexport) void destroyPlugin(IPlugin* p)

#endif // WRITESMART_PLUGIN_INTERFACE_H
