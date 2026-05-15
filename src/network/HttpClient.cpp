#include "network/HttpClient.h"
#include <QJsonDocument>
#include <QSslConfiguration>

HttpClient::HttpClient(QObject* parent)
    : QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);
}

HttpClient::~HttpClient()
{
    cancelAll();
}

void HttpClient::setBaseUrl(const QString& url) { m_baseUrl = url; }
void HttpClient::setApiKey(const QString& key) { m_apiKey = key; }
void HttpClient::setHeaders(const QJsonObject& headers) { m_customHeaders = headers; }
void HttpClient::setTimeout(int secs) { m_timeoutSecs = secs; }

HttpClient* HttpClient::make()
{
    auto* client = new HttpClient();
    // self-destruct when all replies finish and owner doesn't hold reference
    connect(client, &QObject::destroyed, client, &QObject::deleteLater);
    return client;
}

QNetworkRequest HttpClient::buildRequest(const QString& path) const
{
    QUrl url(m_baseUrl + path);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QSslConfiguration ssl = request.sslConfiguration();
    ssl.setProtocol(QSsl::TlsV1_2OrLater);
    request.setSslConfiguration(ssl);

    if (!m_apiKey.isEmpty()) {
        request.setRawHeader("Authorization", ("Bearer " + m_apiKey).toUtf8());
    }
    for (auto it = m_customHeaders.begin(); it != m_customHeaders.end(); ++it) {
        request.setRawHeader(it.key().toUtf8(), it.value().toString().toUtf8());
    }
    return request;
}

void HttpClient::post(const QString& path, const QJsonObject& body, ResponseCallback callback)
{
    QNetworkRequest request = buildRequest(path);
    QJsonDocument doc(body);
    QNetworkReply* reply = m_manager->post(request, doc.toJson(QJsonDocument::Compact));

    connect(reply, &QNetworkReply::finished, this, [this, reply, callback]() {
        finishReply(reply, callback);
    });
}

void HttpClient::postStream(const QString& path, const QJsonObject& body,
                             StreamCallback onChunk, ResponseCallback onDone)
{
    QNetworkRequest request = buildRequest(path);
    QJsonDocument doc(body);

    ActiveRequest ar;
    ar.reply = m_manager->post(request, doc.toJson(QJsonDocument::Compact));
    ar.bytesRead = 0;
    ar.onChunk = onChunk;
    ar.onDone = onDone;

    ar.timer = new QTimer(this);
    ar.timer->setSingleShot(true);
    QTimer* timer = ar.timer;
    QNetworkReply* reply = ar.reply;

    connect(timer, &QTimer::timeout, this, [this, reply]() {
        onTimeout();
    });
    ar.timer->start(m_timeoutSecs * 1000);

    connect(reply, &QNetworkReply::readyRead, this, [this, reply, timer]() {
        timer->start(m_timeoutSecs * 1000);
        onReadyRead();
    });

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onFinished();
    });

    m_activeRequests.append(ar);
}

void HttpClient::get(const QString& path, ResponseCallback callback)
{
    QNetworkRequest request = buildRequest(path);
    QNetworkReply* reply = m_manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, callback]() {
        finishReply(reply, callback);
    });
}

void HttpClient::cancelAll()
{
    for (auto& ar : m_activeRequests) {
        if (ar.timer) { ar.timer->stop(); ar.timer->deleteLater(); }
        if (ar.reply) { ar.reply->abort(); ar.reply->deleteLater(); }
    }
    m_activeRequests.clear();
}

void HttpClient::onReadyRead()
{
    for (int i = 0; i < m_activeRequests.size(); ++i) {
        auto& ar = m_activeRequests[i];
        QNetworkReply* reply = ar.reply;
        if (!reply) continue;

        QByteArray newData = reply->readAll();
        QByteArray incremental = newData.mid(ar.bytesRead);
        ar.bytesRead = newData.size();

        if (!incremental.isEmpty() && ar.onChunk) {
            ar.onChunk(incremental);
        }
    }
}

void HttpClient::onFinished()
{
    for (int i = m_activeRequests.size() - 1; i >= 0; --i) {
        auto& ar = m_activeRequests[i];
        QNetworkReply* reply = ar.reply;
        if (!reply || !reply->isFinished()) continue;

        // 消费剩余未读数据
        if (ar.onChunk) {
            QByteArray tail = reply->readAll();
            if (ar.bytesRead < tail.size()) {
                ar.onChunk(tail.mid(ar.bytesRead));
            }
        }

        if (ar.onDone) {
            int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (reply->error() != QNetworkReply::NoError) {
                ar.onDone(0, QByteArray(), reply->errorString());
            } else {
                ar.onDone(code, QByteArray(), QString());
            }
        }

        if (ar.timer) { ar.timer->stop(); ar.timer->deleteLater(); }
        reply->deleteLater();
        m_activeRequests.removeAt(i);
    }

    // 所有请求完成后自动销毁
    if (m_activeRequests.isEmpty() && parent() == nullptr) {
        deleteLater();
    }
}

void HttpClient::onTimeout()
{
    QNetworkReply* senderReply = nullptr;

    senderReply = static_cast<QNetworkReply*>(sender());
    if (!senderReply) {
        // timer timeout, find which reply timed out
        for (auto& ar : m_activeRequests) {
            if (ar.timer && sender() == ar.timer) {
                senderReply = ar.reply;
                break;
            }
        }
    }
    if (senderReply) senderReply->abort();
}

void HttpClient::finishReply(QNetworkReply* reply, ResponseCallback callback)
{
    int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (reply->error() != QNetworkReply::NoError) {
        callback(0, QByteArray(), reply->errorString());
    } else {
        callback(code, reply->readAll(), QString());
    }
    reply->deleteLater();

    if (m_activeRequests.isEmpty() && parent() == nullptr) {
        deleteLater();
    }
}
