#include "utils/Config.h"
#include "utils/JsonHelper.h"
#include <QFile>

Config& Config::instance()
{
    static Config config;
    return config;
}

bool Config::load(const QString& path)
{
    m_configPath = path;
    QJsonObject obj = JsonHelper::fromFile(path);
    if (obj.isEmpty()) return true;

    if (obj.contains("window")) {
        QJsonObject w = obj["window"].toObject();
        m_windowState.x = JsonHelper::safeGetInt(w, "x", 100);
        m_windowState.y = JsonHelper::safeGetInt(w, "y", 100);
        m_windowState.width = JsonHelper::safeGetInt(w, "width", 1400);
        m_windowState.height = JsonHelper::safeGetInt(w, "height", 900);
        m_windowState.maximized = JsonHelper::safeGetBool(w, "maximized");
        if (w.contains("splitterSizes")) {
            m_windowState.splitterSizes.clear();
            for (const auto& v : w["splitterSizes"].toArray()) {
                m_windowState.splitterSizes.append(v.toInt());
            }
        }
    }

    m_language = JsonHelper::safeGetString(obj, "language", "zh_CN");
    m_theme = JsonHelper::safeGetString(obj, "theme", "default");
    m_fontFamily = JsonHelper::safeGetString(obj, "fontFamily", "Microsoft YaHei");
    m_fontSize = JsonHelper::safeGetInt(obj, "fontSize", 14);
    m_autoSave = JsonHelper::safeGetBool(obj, "autoSave", true);
    m_autoSaveIntervalMs = JsonHelper::safeGetInt(obj, "autoSaveIntervalMs", 2000);
    m_lastDocumentPath = JsonHelper::safeGetString(obj, "lastDocumentPath");

    if (obj.contains("ai")) {
        m_aiConfig = obj["ai"].toObject();
    }

    if (obj.contains("recentDocuments")) {
        for (const auto& v : obj["recentDocuments"].toArray()) {
            m_recentDocuments.append(v.toString());
        }
    }

    return true;
}

bool Config::save(const QString& path)
{
    return JsonHelper::toFile(path.isEmpty() ? m_configPath : path, toJson());
}

WindowState& Config::windowState() { return m_windowState; }
const WindowState& Config::windowState() const { return m_windowState; }

QString Config::language() const { return m_language; }
void Config::setLanguage(const QString& lang) { m_language = lang; }

QString Config::theme() const { return m_theme; }
void Config::setTheme(const QString& theme) { m_theme = theme; }

QString Config::fontFamily() const { return m_fontFamily; }
void Config::setFontFamily(const QString& family) { m_fontFamily = family; }

int Config::fontSize() const { return m_fontSize; }
void Config::setFontSize(int size) { m_fontSize = size; }

bool Config::autoSave() const { return m_autoSave; }
void Config::setAutoSave(bool enabled) { m_autoSave = enabled; }

int Config::autoSaveIntervalMs() const { return m_autoSaveIntervalMs; }
void Config::setAutoSaveIntervalMs(int ms) { m_autoSaveIntervalMs = ms; }

QString Config::lastDocumentPath() const { return m_lastDocumentPath; }
void Config::setLastDocumentPath(const QString& path) { m_lastDocumentPath = path; }

QStringList Config::recentDocuments() const { return m_recentDocuments; }
void Config::addRecentDocument(const QString& path)
{
    m_recentDocuments.removeAll(path);
    m_recentDocuments.prepend(path);
    if (m_recentDocuments.size() > 10) {
        m_recentDocuments = m_recentDocuments.mid(0, 10);
    }
}

QJsonObject Config::aiConfig() const { return m_aiConfig; }
void Config::setAiConfig(const QJsonObject& config) { m_aiConfig = config; }

QJsonObject Config::toJson() const
{
    QJsonObject obj;
    QJsonObject win;
    win["x"] = m_windowState.x;
    win["y"] = m_windowState.y;
    win["width"] = m_windowState.width;
    win["height"] = m_windowState.height;
    win["maximized"] = m_windowState.maximized;
    QJsonArray sizes;
    for (int s : m_windowState.splitterSizes) sizes.append(s);
    win["splitterSizes"] = sizes;
    obj["window"] = win;

    obj["language"] = m_language;
    obj["theme"] = m_theme;
    obj["fontFamily"] = m_fontFamily;
    obj["fontSize"] = m_fontSize;
    obj["autoSave"] = m_autoSave;
    obj["autoSaveIntervalMs"] = m_autoSaveIntervalMs;
    obj["lastDocumentPath"] = m_lastDocumentPath;

    QJsonArray recent;
    for (const auto& d : m_recentDocuments) recent.append(d);
    obj["recentDocuments"] = recent;

    if (!m_aiConfig.isEmpty()) {
        obj["ai"] = m_aiConfig;
    }

    return obj;
}
