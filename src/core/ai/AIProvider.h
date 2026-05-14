#ifndef WRITESMART_AI_PROVIDER_H
#define WRITESMART_AI_PROVIDER_H

#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>
#include <functional>
#include <memory>

struct ChatMessage {
    QString role;       // "system" | "user" | "assistant"
    QString content;
};

struct ChatRequest {
    QString model;
    QList<ChatMessage> messages;
    double temperature = 0.7;
    int maxTokens = 4096;
    QJsonObject extraParams;    // 透传给 API 的额外参数
};

struct ChatResponse {
    QString content;
    QString model;
    int promptTokens = 0;
    int completionTokens = 0;
    QString finishReason;
};

using ChatCallback = std::function<void(bool success, const ChatResponse& response, const QString& error)>;
using StreamCallback = std::function<void(const QString& delta, bool done, const QString& error)>;

class AIProvider {
public:
    virtual ~AIProvider() = default;

    virtual QString providerId() const = 0;
    virtual QString providerName() const = 0;
    virtual QStringList supportedModels() const = 0;

    virtual bool isConfigured() const = 0;
    virtual bool configure(const QJsonObject& config) = 0;
    virtual QJsonObject configuration() const = 0;

    virtual void chat(const ChatRequest& request, ChatCallback callback) = 0;
    virtual void chatStream(const ChatRequest& request, StreamCallback callback) = 0;

    virtual void cancel() = 0;
};

using AIProviderPtr = std::shared_ptr<AIProvider>;
using AIProviderFactory = std::function<AIProviderPtr()>;

#endif // WRITESMART_AI_PROVIDER_H
