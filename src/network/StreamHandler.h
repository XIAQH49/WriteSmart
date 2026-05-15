#ifndef WRITESMART_STREAM_HANDLER_H
#define WRITESMART_STREAM_HANDLER_H

#include <QObject>
#include <QByteArray>
#include <QJsonObject>
#include <functional>

class StreamHandler : public QObject {
    Q_OBJECT

public:
    explicit StreamHandler(QObject* parent = nullptr);

    using TokenCallback = std::function<void(const QString& token)>;
    using ErrorCallback = std::function<void(const QString& error)>;

    void setContentPath(const QString& jsonPath);
    void setTokenCallback(TokenCallback callback);
    void setErrorCallback(ErrorCallback callback);

    void feed(const QByteArray& chunk);
    void reset();

    bool isComplete() const;
    QString finishReason() const;

signals:
    void tokenReceived(const QString& token);
    void streamFinished();

private:
    void parseBuffer();
    QString extractContent(const QJsonObject& obj) const;

    QString m_contentPath = "choices[0].delta.content";
    QByteArray m_buffer;
    QString m_finishReason;
    bool m_complete = false;

    TokenCallback m_tokenCallback;
    ErrorCallback m_errorCallback;
};

#endif
