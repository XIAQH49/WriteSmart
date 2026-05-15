#include "core/ai/providers/ClaudeProvider.h"
#include "network/HttpClient.h"
#include "network/StreamHandler.h"
#include <QJsonDocument>
#include <QObject>

ClaudeProvider::ClaudeProvider() {}

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
    return {
        {"apiKey", m_apiKey},
        {"baseUrl", m_baseUrl},
        {"model", m_model},
        {"temperature", m_temperature},
        {"maxTokens", m_maxTokens}
    };
}

QJsonObject ClaudeProvider::buildRequestBody(const ChatRequest& request) const
{
    QJsonArray messages;
    for (const auto& msg : request.messages) {
        messages.append(QJsonObject{{"role", msg.role}, {"content", msg.content}});
    }

    return {
        {"model", request.model.isEmpty() ? m_model : request.model},
        {"max_tokens", request.maxTokens},
        {"messages", messages}
    };
}

void ClaudeProvider::chat(const ChatRequest& request, ChatCallback callback)
{
    auto* client = HttpClient::make();
    client->setBaseUrl(m_baseUrl);
    QJsonObject headers;
    headers["x-api-key"] = m_apiKey;
    headers["anthropic-version"] = "2023-06-01";
    client->setHeaders(headers);

    client->post("/v1/messages", buildRequestBody(request),
        [callback](int, const QByteArray& data, const QString& error) {
            if (!error.isEmpty()) {
                callback(false, {}, error);
                return;
            }
            QJsonObject obj = QJsonDocument::fromJson(data).object();
            ChatResponse resp;
            if (!obj["content"].toArray().isEmpty()) {
                resp.content = obj["content"].toArray()[0].toObject()["text"].toString();
                resp.model = obj["model"].toString();
            }
            auto usage = obj["usage"].toObject();
            resp.promptTokens = usage["input_tokens"].toInt();
            resp.completionTokens = usage["output_tokens"].toInt();
            resp.finishReason = obj["stop_reason"].toString();
            callback(true, resp, {});
        });
}

void ClaudeProvider::chatStream(const ChatRequest& request, StreamCallback callback)
{
    auto* client = HttpClient::make();
    client->setBaseUrl(m_baseUrl);
    QJsonObject headers;
    headers["x-api-key"] = m_apiKey;
    headers["anthropic-version"] = "2023-06-01";
    client->setHeaders(headers);

    QJsonObject body = buildRequestBody(request);
    body["stream"] = true;

    auto* handler = new StreamHandler(client);
    handler->setContentPath("delta.text");
    handler->setTokenCallback([callback](const QString& token) {
        callback(token, false, {});
    });
    QObject::connect(handler, &StreamHandler::streamFinished, handler, [callback, handler]() {
        callback({}, true, {});
    });

    client->postStream("/v1/messages", body,
        [handler](const QByteArray& chunk) { handler->feed(chunk); },
        [callback, handler](int, const QByteArray&, const QString& error) {
            if (!error.isEmpty()) callback({}, true, error);
        });
}

void ClaudeProvider::cancel() {}
