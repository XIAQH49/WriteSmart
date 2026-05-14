#include "network/HttpClient.h"
#include <QUrlQuery>

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

QNetworkRequest HttpClient::buildRequest(const QString& path) const
{
    QUrl url(m_baseUrl + path);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
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
    m_activeReplies.append(reply);

    connect(reply, &QNetworkReply::finished, this, [this, reply, callback]() {
        m_activeReplies.removeAll(reply);
        int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (reply->error() != QNetworkReply::NoError) {
            callback(0, QByteArray(), reply->errorString());
        } else {
            callback(code, reply->readAll(), QString());
        }
        reply->deleteLater();
    });
}

void HttpClient::postStream(const QString& path, const QJsonObject& body,
                             StreamCallback onChunk, ResponseCallback onDone)
{
    QNetworkRequest request = buildRequest(path);
    QJsonDocument doc(body);
    QNetworkReply* reply = m_manager->post(request, doc.toJson(QJsonDocument::Compact));
    m_activeReplies.append(reply);

    connect(reply, &QNetworkReply::readyRead, this, [reply, onChunk]() {
        onChunk(reply->readAll());
    });

    connect(reply, &QNetworkReply::finished, this, [this, reply, onDone]() {
        m_activeReplies.removeAll(reply);
        int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (reply->error() != QNetworkReply::NoError) {
            onDone(0, QByteArray(), reply->errorString());
        } else {
            onDone(code, QByteArray(), QString());
        }
        reply->deleteLater();
    });
}

void HttpClient::get(const QString& path, ResponseCallback callback)
{
    QNetworkRequest request = buildRequest(path);
    QNetworkReply* reply = m_manager->get(request);
    m_activeReplies.append(reply);

    connect(reply, &QNetworkReply::finished, this, [this, reply, callback]() {
        m_activeReplies.removeAll(reply);
        int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (reply->error() != QNetworkReply::NoError) {
            callback(0, QByteArray(), reply->errorString());
        } else {
            callback(code, reply->readAll(), QString());
        }
        reply->deleteLater();
    });
}

void HttpClient::cancelAll()
{
    for (auto* reply : m_activeReplies) {
        reply->abort();
        reply->deleteLater();
    }
    m_activeReplies.clear();
}

void HttpClient::onReadyRead()
{
}

void HttpClient::onFinished()
{
}
