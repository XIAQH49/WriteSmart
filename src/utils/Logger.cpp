#include "utils/Logger.h"

Logger& Logger::instance()
{
    static Logger logger;
    return logger;
}

void Logger::setLevel(Level level)
{
    m_level = level;
}

void Logger::log(Level level, const QString& message, const char* file, int line)
{
    if (level < m_level) return;

    const char* prefix = "";
    switch (level) {
    case Debug:   prefix = "[DEBUG]"; break;
    case Info:    prefix = "[INFO]";  break;
    case Warning: prefix = "[WARN]";  break;
    case Error:   prefix = "[ERROR]"; break;
    }

    if (file && line > 0) {
        qDebug().noquote() << prefix << QString("%1:%2").arg(file).arg(line) << message;
    } else {
        qDebug().noquote() << prefix << message;
    }
}

void Logger::debug(const QString& msg)   { instance().log(Debug, msg); }
void Logger::info(const QString& msg)    { instance().log(Info, msg); }
void Logger::warning(const QString& msg) { instance().log(Warning, msg); }
void Logger::error(const QString& msg)   { instance().log(Error, msg); }
