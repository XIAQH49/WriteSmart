#ifndef WRITESMART_APPLICATION_H
#define WRITESMART_APPLICATION_H

#include <memory>

class MainWindow;

class Application {
public:
    Application();
    ~Application();

    bool initialize();
    void run();
    void shutdown();

private:
    std::unique_ptr<MainWindow> m_mainWindow;
};

#endif // WRITESMART_APPLICATION_H
