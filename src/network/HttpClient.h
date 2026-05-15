#ifndef WRITESMART_HTTP_CLIENT_H
#define WRITESMART_HTTP_CLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QTimer>
#include <functional>

class HttpClient : public QObject {
    Q_OBJECT

public:
    explicit HttpClient(QObject* parent = nullptr);
    ~HttpClient() override;

    void setBaseUrl(const QString& url);
    void setApiKey(const QString& key);
    void setHeaders(const QJsonObject& headers);
    void setTimeout(int secs);

    using ResponseCallback = std::function<void(int httpCode, const QByteArray& body, const QString& error)>;
    using StreamCallback = std::function<void(const QByteArray& chunk)>;

    void post(const QString& path, const QJsonObject& body, ResponseCallback callback);
    void postStream(const QString& path, const QJsonObject& body,
                    StreamCallback onChunk, ResponseCallback onDone);
    void get(const QString& path, ResponseCallback callback);
    void cancelAll();

    static HttpClient* make();  // 工厂：返回 self-deleting 实例

private slots:
    void onReadyRead();
    void onFinished();
    void onTimeout();

private:
    QNetworkRequest buildRequest(const QString& path) const;
    void finishReply(QNetworkReply* reply, ResponseCallback callback);

    QNetworkAccessManager* m_manager = nullptr;
    QString m_baseUrl;
    QString m_apiKey;
    QJsonObject m_customHeaders;
    int m_timeoutSecs = 30;

    struct ActiveRequest {
        QNetworkReply* reply = nullptr;
        qint64 bytesRead = 0;
        StreamCallback onChunk;
        ResponseCallback onDone;
        QTimer* timer = nullptr;
    };
    QList<ActiveRequest> m_activeRequests;
};

#endif
