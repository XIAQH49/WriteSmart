#ifndef WRITESMART_THEME_MANAGER_H
#define WRITESMART_THEME_MANAGER_H

#include <QString>
#include <QMap>

class ThemeManager {
public:
    static ThemeManager& instance();

    bool loadTheme(const QString& name);
    QString currentTheme() const;
    QStringList availableThemes() const;

    QString styleSheet() const;

private:
    ThemeManager() = default;

    QString m_currentTheme = "default";
    QString m_styleSheet;
};

#endif // WRITESMART_THEME_MANAGER_H
