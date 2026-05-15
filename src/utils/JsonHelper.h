#ifndef WRITESMART_JSON_HELPER_H
#define WRITESMART_JSON_HELPER_H

#include <QJsonObject>
#include <QJsonArray>
#include <QString>

class JsonHelper {
public:
    static QString toString(const QJsonObject& obj, bool compact = false);
    static QJsonObject fromString(const QString& json);
    static QJsonObject fromFile(const QString& path);
    static bool toFile(const QString& path, const QJsonObject& obj);

    static QString safeGetString(const QJsonObject& obj, const QString& key,
                                  const QString& defaultValue = QString());
    static int safeGetInt(const QJsonObject& obj, const QString& key, int defaultValue = 0);
    static double safeGetDouble(const QJsonObject& obj, const QString& key,
                                 double defaultValue = 0.0);
    static bool safeGetBool(const QJsonObject& obj, const QString& key, bool defaultValue = false);
};

#endif // WRITESMART_JSON_HELPER_H
