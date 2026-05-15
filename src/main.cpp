#include <QApplication>
#include <QDir>
#include "utils/Config.h"
#include "app/Application.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("WriteSmart");
    app.setApplicationVersion("2.0.0");
    app.setOrganizationName("WriteSmart");

    QString configPath = QDir(QApplication::applicationDirPath())
                             .filePath("config.json");
    Config::instance().load(configPath);

    Application wsApp;
    wsApp.initialize();
    wsApp.run();

    int result = app.exec();

    Config::instance().save(configPath);

    return result;
}
