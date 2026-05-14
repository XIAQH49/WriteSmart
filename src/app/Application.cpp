#include "app/Application.h"
#include "ui/MainWindow.h"
#include "core/ai/providers/OpenAIProvider.h"
#include "core/ai/providers/ClaudeProvider.h"
#include "core/ai/providers/CustomProvider.h"
#include "utils/Logger.h"

Application::Application() = default;

Application::~Application()
{
    shutdown();
}

bool Application::initialize()
{
    Logger::info("WriteSmart 2.0 initializing...");

    m_mainWindow = std::make_unique<MainWindow>();

    Logger::info("Application initialized successfully");
    return true;
}

void Application::run()
{
    if (m_mainWindow) {
        m_mainWindow->show();
    }
}

void Application::shutdown()
{
    Logger::info("WriteSmart shutting down...");
    m_mainWindow.reset();
}
