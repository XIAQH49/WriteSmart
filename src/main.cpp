#include <QApplication>
#include <QDir>
#include <QCommandLineParser>
#include <QFile>
#include "utils/Config.h"
#include "app/Application.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("WriteSmart");
    app.setApplicationVersion("2.0.0");
    app.setOrganizationName("WriteSmart");

    QCommandLineParser parser;
    parser.setApplicationDescription("智能文学创作客户端");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "打开指定的文档文件");
    parser.process(app);

    QString configPath = QDir(QApplication::applicationDirPath())
                             .filePath("config.json");
    Config::instance().load(configPath);

    Application wsApp;
    wsApp.initialize();

    // CLI 传入文件路径 → 直接打开
    const QStringList positionalArgs = parser.positionalArguments();
    if (!positionalArgs.isEmpty() && QFile::exists(positionalArgs.first())) {
        wsApp.restoreSession(positionalArgs.first());
    }

    wsApp.run();

    int result = app.exec();

    Config::instance().save(configPath);

    return result;
}
