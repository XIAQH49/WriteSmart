#include "utils/JsonHelper.h"
#include <QFile>
#include <QJsonDocument>

QString JsonHelper::toString(const QJsonObject& obj, bool compact)
{
    QJsonDocument doc(obj);
    return QString::fromUtf8(compact ? doc.toJson(QJsonDocument::Compact)
                                     : doc.toJson(QJsonDocument::Indented));
}

QJsonObject JsonHelper::fromString(const QString& json)
{
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    return doc.object();
}

QJsonObject JsonHelper::fromFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return {};
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    return doc.object();
}

bool JsonHelper::toFile(const QString& path, const QJsonObject& obj)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) return false;
    file.write(toString(obj, false).toUtf8());
    file.close();
    return true;
}

QString JsonHelper::safeGetString(const QJsonObject& obj, const QString& key, const QString& defaultValue)
{
    return obj.contains(key) ? obj[key].toString() : defaultValue;
}

int JsonHelper::safeGetInt(const QJsonObject& obj, const QString& key, int defaultValue)
{
    return obj.contains(key) ? obj[key].toInt() : defaultValue;
}

double JsonHelper::safeGetDouble(const QJsonObject& obj, const QString& key, double defaultValue)
{
    return obj.contains(key) ? obj[key].toDouble() : defaultValue;
}

bool JsonHelper::safeGetBool(const QJsonObject& obj, const QString& key, bool defaultValue)
{
    return obj.contains(key) ? obj[key].toBool() : defaultValue;
}
