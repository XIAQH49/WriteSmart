#include "app/Application.h"
#include "ui/MainWindow.h"
#include "core/ai/providers/OpenAIProvider.h"
#include "core/ai/providers/ClaudeProvider.h"
#include "core/ai/providers/CustomProvider.h"
#include "core/plugin/PluginManager.h"
#include "utils/Logger.h"
#include "utils/Config.h"
#include <QDir>
#include <QApplication>

Application::Application() = default;

Application::~Application()
{
    shutdown();
}

bool Application::initialize()
{
    Logger::info("WriteSmart 2.0 initializing...");

    // 扫描插件目录
    QString pluginDir = QDir(QApplication::applicationDirPath()).filePath("plugins");
    PluginManager::instance().scanPlugins(pluginDir);
    Logger::info(QString("Loaded %1 plugins").arg(PluginManager::instance().loadedPlugins().size()));

    // 恢复上次文档
    m_mainWindow = std::make_unique<MainWindow>();
    m_mainWindow->show();

    restoreSession();

    Logger::info("Application initialized successfully");
    return true;
}

void Application::run()
{
    if (m_mainWindow) m_mainWindow->show();
}

void Application::shutdown()
{
    Logger::info("WriteSmart shutting down...");
    saveSession();
    PluginManager::instance().unloadAll();
    m_mainWindow.reset();
}

void Application::restoreSession()
{
    const QString& lastPath = Config::instance().lastDocumentPath();
    if (!lastPath.isEmpty()) {
        m_mainWindow->openDocument(lastPath);
    }
}

void Application::saveSession()
{
    if (m_mainWindow) {
        m_mainWindow->saveDocumentIfNeeded();
    }
}
