#ifndef WRITESMART_CONFIG_H
#define WRITESMART_CONFIG_H

#include <QString>
#include <QSize>
#include <QPoint>
#include <QJsonObject>
#include <QList>

struct WindowState {
    int x = 100;
    int y = 100;
    int width = 1400;
    int height = 900;
    bool maximized = false;
    QList<int> splitterSizes = {220, 830, 350};
};

class Config {
public:
    static Config& instance();

    bool load(const QString& path);
    bool save(const QString& path);

    WindowState& windowState();
    const WindowState& windowState() const;

    QString language() const;
    void setLanguage(const QString& lang);

    QString theme() const;
    void setTheme(const QString& theme);

    QString fontFamily() const;
    void setFontFamily(const QString& family);

    int fontSize() const;
    void setFontSize(int size);

    bool autoSave() const;
    void setAutoSave(bool enabled);

    int autoSaveIntervalMs() const;
    void setAutoSaveIntervalMs(int ms);

    QString lastDocumentPath() const;
    void setLastDocumentPath(const QString& path);

    QStringList recentDocuments() const;
    void addRecentDocument(const QString& path);

    QJsonObject toJson() const;

private:
    Config() = default;
    QString m_configPath;
    WindowState m_windowState;
    QString m_language = "zh_CN";
    QString m_theme = "default";
    QString m_fontFamily = "Microsoft YaHei";
    int m_fontSize = 14;
    bool m_autoSave = true;
    int m_autoSaveIntervalMs = 2000;
    QString m_lastDocumentPath;
    QStringList m_recentDocuments;
};

#endif // WRITESMART_CONFIG_H
