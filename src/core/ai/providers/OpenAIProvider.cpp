#include "core/ai/providers/OpenAIProvider.h"
#include "network/HttpClient.h"
#include "network/StreamHandler.h"
#include <QJsonDocument>
#include <QObject>

OpenAIProvider::OpenAIProvider() {}

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
    return {
        {"apiKey", m_apiKey},
        {"baseUrl", m_baseUrl},
        {"model", m_model},
        {"temperature", m_temperature},
        {"maxTokens", m_maxTokens}
    };
}

QJsonObject OpenAIProvider::buildRequestBody(const ChatRequest& request) const
{
    QJsonArray messages;
    for (const auto& msg : request.messages) {
        messages.append(QJsonObject{{"role", msg.role}, {"content", msg.content}});
    }

    return {
        {"model", request.model.isEmpty() ? m_model : request.model},
        {"temperature", request.temperature},
        {"max_tokens", request.maxTokens},
        {"messages", messages}
    };
}

void OpenAIProvider::chat(const ChatRequest& request, ChatCallback callback)
{
    auto* client = HttpClient::make();
    client->setBaseUrl(m_baseUrl);
    client->setApiKey(m_apiKey);

    client->post("/v1/chat/completions", buildRequestBody(request),
        [callback](int /*code*/, const QByteArray& data, const QString& error) {
            if (!error.isEmpty()) {
                callback(false, {}, error);
                return;
            }
            QJsonObject obj = QJsonDocument::fromJson(data).object();
            ChatResponse resp;
            if (!obj["choices"].toArray().isEmpty()) {
                auto choice = obj["choices"].toArray()[0].toObject();
                resp.content = choice["message"].toObject()["content"].toString();
                resp.model = obj["model"].toString();
            }
            auto usage = obj["usage"].toObject();
            resp.promptTokens = usage["prompt_tokens"].toInt();
            resp.completionTokens = usage["completion_tokens"].toInt();
            resp.finishReason = obj["choices"].toArray()[0].toObject()["finish_reason"].toString();
            callback(true, resp, {});
        });
}

void OpenAIProvider::chatStream(const ChatRequest& request, StreamCallback callback)
{
    auto* client = HttpClient::make();
    client->setBaseUrl(m_baseUrl);
    client->setApiKey(m_apiKey);

    QJsonObject body = buildRequestBody(request);
    body["stream"] = true;

    auto* handler = new StreamHandler(client);
    handler->setTokenCallback([callback](const QString& token) {
        callback(token, false, {});
    });
    QObject::connect(handler, &StreamHandler::streamFinished, handler, [callback, handler]() {
        callback({}, true, {});
    });

    client->postStream("/v1/chat/completions", body,
        [handler](const QByteArray& chunk) { handler->feed(chunk); },
        [callback, handler](int, const QByteArray&, const QString& error) {
            if (!error.isEmpty()) callback({}, true, error);
        });
}

void OpenAIProvider::cancel() {}
