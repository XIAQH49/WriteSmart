#include "core/ai/providers/OpenAIProvider.h"
#include "network/HttpClient.h"
#include "network/StreamHandler.h"
#include "utils/JsonHelper.h"

OpenAIProvider::OpenAIProvider()
{
    m_network = new QNetworkAccessManager();
}

QString OpenAIProvider::providerId() const { return "openai"; }
QString OpenAIProvider::providerName() const { return "OpenAI"; }

QStringList OpenAIProvider::supportedModels() const
{
    return {"gpt-4o", "gpt-4o-mini", "gpt-4-turbo", "gpt-3.5-turbo"};
}

bool OpenAIProvider::isConfigured() const { return m_configured; }

bool OpenAIProvider::configure(const QJsonObject& config)
{
    m_apiKey = config["apiKey"].toString();
    m_baseUrl = config["baseUrl"].toString("https://api.openai.com");
    m_model = config["model"].toString("gpt-4o");
    m_temperature = config["temperature"].toDouble(0.7);
    m_maxTokens = config["maxTokens"].toInt(4096);
    m_configured = !m_apiKey.isEmpty();
    return m_configured;
}

QJsonObject OpenAIProvider::configuration() const
{
    QJsonObject obj;
    obj["apiKey"] = m_apiKey;
    obj["baseUrl"] = m_baseUrl;
    obj["model"] = m_model;
    obj["temperature"] = m_temperature;
    obj["maxTokens"] = m_maxTokens;
    return obj;
}

QJsonObject OpenAIProvider::buildRequestBody(const ChatRequest& request) const
{
    QJsonObject body;
    body["model"] = request.model.isEmpty() ? m_model : request.model;
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

void OpenAIProvider::chat(const ChatRequest& request, ChatCallback callback)
{
    auto* client = new HttpClient();
    client->setBaseUrl(m_baseUrl);
    client->setApiKey(m_apiKey);

    QJsonObject body = buildRequestBody(request);
    client->post("/v1/chat/completions", body,
        [callback](int /*code*/, const QByteArray& data, const QString& error) {
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
            resp.promptTokens = obj["usage"].toObject()["prompt_tokens"].toInt();
            resp.completionTokens = obj["usage"].toObject()["completion_tokens"].toInt();
            callback(true, resp, {});
        });
}

void OpenAIProvider::chatStream(const ChatRequest& request, StreamCallback callback)
{
    auto* client = new HttpClient();
    client->setBaseUrl(m_baseUrl);
    client->setApiKey(m_apiKey);

    QJsonObject body = buildRequestBody(request);
    body["stream"] = true;

    auto* handler = new StreamHandler();
    handler->setTokenCallback([callback](const QString& token) {
        callback(token, false, {});
    });

    client->postStream("/v1/chat/completions", body,
        [handler](const QByteArray& chunk) {
            handler->feed(chunk);
        },
        [callback, handler](int, const QByteArray&, const QString& error) {
            if (!error.isEmpty()) {
                callback({}, true, error);
            } else {
                callback({}, true, {});
            }
            handler->deleteLater();
        });
}

void OpenAIProvider::cancel()
{
    // Network requests are managed per-call via HttpClient
}
