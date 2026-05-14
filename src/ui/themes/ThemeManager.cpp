#include "ui/themes/ThemeManager.h"
#include <QFile>
#include <QDir>

ThemeManager& ThemeManager::instance()
{
    static ThemeManager mgr;
    return mgr;
}

bool ThemeManager::loadTheme(const QString& name)
{
    QString path = ":/styles/" + name + ".qss";
    if (!name.isEmpty() && !name.endsWith(".qss")) {
        path = name;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return false;
    m_styleSheet = QString::fromUtf8(file.readAll());
    file.close();
    m_currentTheme = name;
    return true;
}

QString ThemeManager::currentTheme() const { return m_currentTheme; }

QStringList ThemeManager::availableThemes() const { return {"default"}; }

QString ThemeManager::styleSheet() const { return m_styleSheet; }
