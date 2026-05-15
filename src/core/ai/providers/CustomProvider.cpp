#include "core/ai/providers/CustomProvider.h"
#include "network/HttpClient.h"
#include "network/StreamHandler.h"
#include <QJsonDocument>
#include <QObject>

CustomProvider::CustomProvider() {}

QString CustomProvider::providerId() const { return "custom"; }
QString CustomProvider::providerName() const { return "Custom API"; }

QStringList CustomProvider::supportedModels() const { return {m_endpoint.model}; }

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
    return {
        {"baseUrl", m_endpoint.baseUrl},
        {"chatPath", m_endpoint.chatPath},
        {"apiKey", m_endpoint.apiKey},
        {"model", m_endpoint.model},
        {"responseContentPath", m_endpoint.responseContentPath}
    };
}

QJsonObject CustomProvider::buildRequestBody(const ChatRequest& request) const
{
    if (!m_endpoint.requestTemplate.isEmpty()) {
        QJsonObject body = m_endpoint.requestTemplate;

        // 变量替换
        QJsonArray messages;
        for (const auto& msg : request.messages) {
            messages.append(QJsonObject{{"role", msg.role}, {"content", msg.content}});
        }

        QString raw = QJsonDocument(body).toJson();
        raw.replace("{model}", request.model.isEmpty() ? m_endpoint.model : request.model);
        raw.replace("{temperature}", QString::number(request.temperature));
        raw.replace("{max_tokens}", QString::number(request.maxTokens));

        // messages as raw JSON array
        QString msgsJson = QString::fromUtf8(QJsonDocument(messages).toJson(QJsonDocument::Compact));
        raw.replace("{messages}", msgsJson);
        // fallback: replace the placeholder object
        raw.replace("\"{messages}\"", msgsJson);

        QJsonParseError err;
        return QJsonDocument::fromJson(raw.toUtf8(), &err).object();
    }

    QJsonArray messages;
    for (const auto& msg : request.messages) {
        messages.append(QJsonObject{{"role", msg.role}, {"content", msg.content}});
    }

    return {
        {"model", request.model.isEmpty() ? m_endpoint.model : request.model},
        {"temperature", request.temperature},
        {"max_tokens", request.maxTokens},
        {"messages", messages}
    };
}

void CustomProvider::chat(const ChatRequest& request, ChatCallback callback)
{
    auto* client = HttpClient::make();
    client->setBaseUrl(m_endpoint.baseUrl);
    client->setApiKey(m_endpoint.apiKey);
    client->setHeaders(m_endpoint.headers);
    client->setTimeout(m_endpoint.timeoutSecs);

    client->post(m_endpoint.chatPath, buildRequestBody(request),
        [callback](int, const QByteArray& data, const QString& error) {
            if (!error.isEmpty()) { callback(false, {}, error); return; }
            ChatResponse resp;
            QJsonObject obj = QJsonDocument::fromJson(data).object();
            if (!obj["choices"].toArray().isEmpty()) {
                resp.content = obj["choices"].toArray()[0].toObject()["message"].toObject()["content"].toString();
                resp.model = obj["model"].toString();
            }
            callback(true, resp, {});
        });
}

void CustomProvider::chatStream(const ChatRequest& request, StreamCallback callback)
{
    auto* client = HttpClient::make();
    client->setBaseUrl(m_endpoint.baseUrl);
    client->setApiKey(m_endpoint.apiKey);
    client->setHeaders(m_endpoint.headers);
    client->setTimeout(m_endpoint.timeoutSecs);

    QJsonObject body = buildRequestBody(request);
    body["stream"] = true;

    auto* handler = new StreamHandler(client);
    handler->setContentPath(m_endpoint.responseContentPath);
    handler->setTokenCallback([callback](const QString& token) {
        callback(token, false, {});
    });
    QObject::connect(handler, &StreamHandler::streamFinished, handler, [callback, handler]() {
        callback({}, true, {});
    });

    client->postStream(m_endpoint.chatPath, body,
        [handler](const QByteArray& chunk) { handler->feed(chunk); },
        [callback, handler](int, const QByteArray&, const QString& error) {
            if (!error.isEmpty()) callback({}, true, error);
        });
}

void CustomProvider::cancel() {}
