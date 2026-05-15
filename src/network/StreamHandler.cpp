#include "network/StreamHandler.h"
#include <QJsonDocument>
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
    m_buffer += chunk;
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
    // SSE 格式: "data: {...}\n\n" 或 "data: {...}\r\n\r\n"
    static const QByteArray sep1("\n\n");
    static const QByteArray sep2("\r\n\r\n");

    while (true) {
        int idx = m_buffer.indexOf(sep1);
        int sepLen = 2;
        if (idx < 0) {
            idx = m_buffer.indexOf(sep2);
            sepLen = 4;
        }
        if (idx < 0) break;

        QByteArray block = m_buffer.left(idx);
        m_buffer = m_buffer.mid(idx + sepLen);

        // 按行分割每个 field
        const auto& lines = block.split('\n');
        for (const QByteArray& rawLine : lines) {
            QByteArray line = rawLine.trimmed();
            if (line.isEmpty() || line.startsWith(':')) continue; // comment

            if (line.startsWith("data:")) {
                QByteArray data = line.mid(5).trimmed();

                if (data == "[DONE]") {
                    m_complete = true;
                    continue;
                }

                QJsonParseError err;
                QJsonDocument doc = QJsonDocument::fromJson(data, &err);
                if (err.error != QJsonParseError::NoError) continue;

                QJsonObject root = doc.object();
                QJsonObject objToParse = root;

                // Anthropic 格式: {type: "content_block_delta", delta: {text: "..."}}
                if (root.contains("type") && root["type"].toString() == "content_block_delta") {
                    objToParse = root["delta"].toObject();
                }

                QString content = extractContent(objToParse);
                if (!content.isEmpty()) {
                    if (m_tokenCallback) m_tokenCallback(content);
                    emit tokenReceived(content);
                }

                // check finish reason
                static const QStringList finishKeys = {"finish_reason", "stop_reason"};
                for (const auto& key : finishKeys) {
                    QString reason;
                    if (root.contains(key)) {
                        reason = root[key].toString();
                    } else if (root.contains("choices") && !root["choices"].toArray().isEmpty()) {
                        auto choice = root["choices"].toArray()[0].toObject();
                        reason = choice[key].toString();
                    }
                    if (!reason.isEmpty()) {
                        m_finishReason = reason;
                        m_complete = true;
                    }
                }
            } else if (line.startsWith("event:")) {
                // handle event type if needed
                QByteArray event = line.mid(6).trimmed();
                if (event == "done" || event == "message_stop") {
                    m_complete = true;
                }
            }
        }
    }

    if (m_complete) emit streamFinished();
}

QString StreamHandler::extractContent(const QJsonObject& obj) const
{
    QString path = m_contentPath;

    // 优先直接匹配 key
    if (path.contains('.')) {
        QStringList keys;
        for (const QString& part : path.split('.')) {
            QString clean = part.trimmed();
            if (clean.isEmpty()) continue;

            int bracket = clean.indexOf('[');
            if (bracket >= 0) {
                QString baseKey = clean.left(bracket);
                keys.append(baseKey);
                int endBracket = clean.indexOf(']', bracket);
                if (endBracket > bracket) {
                    keys.append(clean.mid(bracket + 1, endBracket - bracket - 1));
                }
            } else {
                keys.append(clean);
            }
        }

        QJsonValue current(obj);
        for (const QString& key : keys) {
            if (current.isArray()) {
                bool ok;
                int idx = key.toInt(&ok);
                if (ok && idx < current.toArray().size()) {
                    current = current.toArray()[idx];
                } else {
                    return {};
                }
            } else if (current.isObject()) {
                current = current.toObject()[key];
            } else {
                return {};
            }
        }
        return current.toString();
    }

    // simple direct key
    if (path == "delta.text") {
        return obj["delta"].toObject()["text"].toString();
    }
    return obj[path].toString();
}
