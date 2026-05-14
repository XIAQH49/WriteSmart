#ifndef WRITESMART_LOGGER_H
#define WRITESMART_LOGGER_H

#include <QString>
#include <QDebug>

class Logger {
public:
    enum Level { Debug, Info, Warning, Error };

    static Logger& instance();

    void setLevel(Level level);
    void log(Level level, const QString& message, const char* file = nullptr, int line = 0);

    static void debug(const QString& msg);
    static void info(const QString& msg);
    static void warning(const QString& msg);
    static void error(const QString& msg);

private:
    Logger() = default;
    Level m_level = Debug;
};

#endif // WRITESMART_LOGGER_H
