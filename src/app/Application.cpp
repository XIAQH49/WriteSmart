#include "app/Application.h"
#include "ui/MainWindow.h"
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

    QString pluginDir = QDir(QApplication::applicationDirPath()).filePath("plugins");
    PluginManager::instance().scanPlugins(pluginDir);
    Logger::info(QString("Loaded %1 plugins").arg(PluginManager::instance().loadedPlugins().size()));

    m_mainWindow = std::make_unique<MainWindow>();

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

void Application::restoreSession(const QString& filePath)
{
    if (!m_mainWindow) return;

    if (!filePath.isEmpty()) {
        m_mainWindow->openDocument(filePath);
        return;
    }

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
