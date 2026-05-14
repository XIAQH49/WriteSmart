#include "core/ai/providers/CustomProvider.h"
#include "network/HttpClient.h"
#include "network/StreamHandler.h"
#include "utils/JsonHelper.h"

CustomProvider::CustomProvider()
{
    m_network = new QNetworkAccessManager();
}

QString CustomProvider::providerId() const { return "custom"; }
QString CustomProvider::providerName() const { return "Custom API"; }

QStringList CustomProvider::supportedModels() const
{
    return {m_endpoint.model};
}

bool CustomProvider::isConfigured() const { return m_configured; }

bool CustomProvider::configure(const QJsonObject& config)
{
    m_endpoint.baseUrl = config["baseUrl"].toString();
    m_endpoint.chatPath = config["chatPath"].toString("/v1/chat/completions");
    m_endpoint.apiKey = config["apiKey"].toString();
    m_endpoint.model = config["model"].toString();
    m_endpoint.requestTemplate = config["requestTemplate"].toObject();
    m_endpoint.responseContentPath = config["responseContentPath"].toString("choices[0].message.content");
    m_endpoint.temperature = config["temperature"].toDouble(0.7);
    m_endpoint.maxTokens = config["maxTokens"].toInt(4096);
    m_endpoint.timeoutSecs = config["timeoutSecs"].toInt(30);
    m_endpoint.headers = config["headers"].toObject();
    m_configured = !m_endpoint.baseUrl.isEmpty();
    return m_configured;
}

QJsonObject CustomProvider::configuration() const
{
    QJsonObject obj;
    obj["baseUrl"] = m_endpoint.baseUrl;
    obj["chatPath"] = m_endpoint.chatPath;
    obj["apiKey"] = m_endpoint.apiKey;
    obj["model"] = m_endpoint.model;
    obj["requestTemplate"] = m_endpoint.requestTemplate;
    obj["responseContentPath"] = m_endpoint.responseContentPath;
    return obj;
}

QJsonObject CustomProvider::buildRequestBody(const ChatRequest& request) const
{
    if (!m_endpoint.requestTemplate.isEmpty()) {
        QJsonObject body;
        for (auto it = m_endpoint.requestTemplate.begin(); it != m_endpoint.requestTemplate.end(); ++it) {
            body[it.key()] = it.value();
        }
        return body;
    }

    QJsonObject body;
    body["model"] = request.model.isEmpty() ? m_endpoint.model : request.model;
    body["temperature"] = request.temperature;
    body["max_tokens"] = request.maxTokens;

    QJsonArray messages;
    for (const auto& msg : request.messages) {
        QJsonObject m;
        m["role"] = msg.role;
        m["content"] = msg.content;
        messages.append(m);
    }
    body["messages"] = messages;
    return body;
}

void CustomProvider::chat(const ChatRequest& request, ChatCallback callback)
{
    auto* client = new HttpClient();
    client->setBaseUrl(m_endpoint.baseUrl);
    client->setApiKey(m_endpoint.apiKey);
    client->setHeaders(m_endpoint.headers);
    client->setTimeout(m_endpoint.timeoutSecs);

    QJsonObject body = buildRequestBody(request);
    client->post(m_endpoint.chatPath, body,
        [this, callback](int, const QByteArray& data, const QString& error) {
            if (!error.isEmpty()) {
                callback(false, {}, error);
                return;
            }
            ChatResponse resp;
            QJsonObject obj = QJsonDocument::fromJson(data).object();
            if (obj.contains("choices") && !obj["choices"].toArray().isEmpty()) {
                auto choice = obj["choices"].toArray()[0].toObject();
                resp.content = choice["message"].toObject()["content"].toString();
                resp.model = obj["model"].toString();
            }
            callback(true, resp, {});
        });
}

void CustomProvider::chatStream(const ChatRequest& request, StreamCallback callback)
{
    auto* client = new HttpClient();
    client->setBaseUrl(m_endpoint.baseUrl);
    client->setApiKey(m_endpoint.apiKey);
    client->setHeaders(m_endpoint.headers);
    client->setTimeout(m_endpoint.timeoutSecs);

    QJsonObject body = buildRequestBody(request);
    body["stream"] = true;

    auto* handler = new StreamHandler();
    handler->setContentPath(m_endpoint.responseContentPath);
    handler->setTokenCallback([callback](const QString& token) {
        callback(token, false, {});
    });

    client->postStream(m_endpoint.chatPath, body,
        [handler](const QByteArray& chunk) { handler->feed(chunk); },
        [callback, handler](int, const QByteArray&, const QString& error) {
            if (!error.isEmpty()) callback({}, true, error);
            else callback({}, true, {});
            handler->deleteLater();
        });
}

void CustomProvider::cancel() {}
