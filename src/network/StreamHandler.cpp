#include "network/StreamHandler.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>

StreamHandler::StreamHandler(QObject* parent)
    : QObject(parent)
{
}

void StreamHandler::setContentPath(const QString& jsonPath) { m_contentPath = jsonPath; }
void StreamHandler::setTokenCallback(TokenCallback callback) { m_tokenCallback = callback; }
void StreamHandler::setErrorCallback(ErrorCallback callback) { m_errorCallback = callback; }

void StreamHandler::feed(const QByteArray& chunk)
{
    m_buffer += QString::fromUtf8(chunk);
    parseBuffer();
}

void StreamHandler::reset()
{
    m_buffer.clear();
    m_finishReason.clear();
    m_complete = false;
}

bool StreamHandler::isComplete() const { return m_complete; }
QString StreamHandler::finishReason() const { return m_finishReason; }

void StreamHandler::parseBuffer()
{
    while (true) {
        int idx = m_buffer.indexOf("\n\n");
        if (idx < 0) break;

        QString line = m_buffer.left(idx);
        m_buffer = m_buffer.mid(idx + 2);

        if (line.startsWith("data: ")) {
            QString data = line.mid(6).trimmed();
            if (data == "[DONE]") {
                m_complete = true;
                continue;
            }

            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8(), &err);
            if (err.error == QJsonParseError::NoError) {
                if (doc.object().contains("choices")) {
                    QString content = extractContent(doc.object());
                    if (!content.isEmpty() && m_tokenCallback) {
                        m_tokenCallback(content);
                    }
                    QString reason = doc.object()["choices"].toArray()[0].toObject()["finish_reason"].toString();
                    if (!reason.isEmpty()) {
                        m_finishReason = reason;
                        m_complete = true;
                    }
                }
            }
        }
    }
}

QString StreamHandler::extractContent(const QJsonObject& obj) const
{
    QString path = m_contentPath;
    path.replace('[', '.');
    path.replace(']', ' ');

    QStringList keys = path.split('.', Qt::SkipEmptyParts);
    QJsonValue current(obj);

    for (const QString& key : keys) {
        QString cleanKey = key.trimmed();
        if (cleanKey.isEmpty()) continue;

        if (current.isArray()) {
            bool ok;
            int idx = cleanKey.toInt(&ok);
            if (ok && idx < current.toArray().size()) {
                current = current.toArray()[idx];
            } else {
                return {};
            }
        } else if (current.isObject()) {
            current = current.toObject()[cleanKey];
        } else {
            return {};
        }
    }

    return current.toString();
}
