#ifndef WRITESMART_APPLICATION_H
#define WRITESMART_APPLICATION_H

#include <memory>
#include <QString>

class MainWindow;

class Application {
public:
    Application();
    ~Application();

    bool initialize();
    void run();
    void shutdown();
    void restoreSession(const QString& filePath = QString());

private:
    void saveSession();

    std::unique_ptr<MainWindow> m_mainWindow;
};

#endif
