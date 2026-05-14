#include "core/ai/providers/ClaudeProvider.h"
#include "network/HttpClient.h"
#include "network/StreamHandler.h"

ClaudeProvider::ClaudeProvider()
{
    m_network = new QNetworkAccessManager();
}

QString ClaudeProvider::providerId() const { return "claude"; }
QString ClaudeProvider::providerName() const { return "Anthropic Claude"; }

QStringList ClaudeProvider::supportedModels() const
{
    return {"claude-3-5-sonnet-20241022", "claude-3-opus-20240229", "claude-3-haiku-20240307"};
}

bool ClaudeProvider::isConfigured() const { return m_configured; }

bool ClaudeProvider::configure(const QJsonObject& config)
{
    m_apiKey = config["apiKey"].toString();
    m_baseUrl = config["baseUrl"].toString("https://api.anthropic.com");
    m_model = config["model"].toString("claude-3-5-sonnet-20241022");
    m_temperature = config["temperature"].toDouble(0.7);
    m_maxTokens = config["maxTokens"].toInt(4096);
    m_configured = !m_apiKey.isEmpty();
    return m_configured;
}

QJsonObject ClaudeProvider::configuration() const
{
    QJsonObject obj;
    obj["apiKey"] = m_apiKey;
    obj["baseUrl"] = m_baseUrl;
    obj["model"] = m_model;
    obj["temperature"] = m_temperature;
    obj["maxTokens"] = m_maxTokens;
    return obj;
}

QJsonObject ClaudeProvider::buildRequestBody(const ChatRequest& request) const
{
    QJsonObject body;
    body["model"] = request.model.isEmpty() ? m_model : request.model;
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

void ClaudeProvider::chat(const ChatRequest& request, ChatCallback callback)
{
    auto* client = new HttpClient();
    client->setBaseUrl(m_baseUrl);
    client->setApiKey(m_apiKey);
    QJsonObject headers;
    headers["x-api-key"] = m_apiKey;
    headers["anthropic-version"] = "2023-06-01";
    client->setHeaders(headers);

    QJsonObject body = buildRequestBody(request);
    client->post("/v1/messages", body,
        [callback](int, const QByteArray& data, const QString& error) {
            if (!error.isEmpty()) {
                callback(false, {}, error);
                return;
            }
            ChatResponse resp;
            QJsonObject obj = QJsonDocument::fromJson(data).object();
            if (obj.contains("content") && !obj["content"].toArray().isEmpty()) {
                resp.content = obj["content"].toArray()[0].toObject()["text"].toString();
                resp.model = obj["model"].toString();
            }
            callback(true, resp, {});
        });
}

void ClaudeProvider::chatStream(const ChatRequest& request, StreamCallback callback)
{
    auto* client = new HttpClient();
    client->setBaseUrl(m_baseUrl);
    client->setApiKey(m_apiKey);
    QJsonObject headers;
    headers["x-api-key"] = m_apiKey;
    headers["anthropic-version"] = "2023-06-01";
    client->setHeaders(headers);

    QJsonObject body = buildRequestBody(request);
    body["stream"] = true;

    auto* handler = new StreamHandler();
    handler->setContentPath("delta.text");
    handler->setTokenCallback([callback](const QString& token) {
        callback(token, false, {});
    });

    client->postStream("/v1/messages", body,
        [handler](const QByteArray& chunk) { handler->feed(chunk); },
        [callback, handler](int, const QByteArray&, const QString& error) {
            if (!error.isEmpty()) callback({}, true, error);
            else callback({}, true, {});
            handler->deleteLater();
        });
}

void ClaudeProvider::cancel() {}
