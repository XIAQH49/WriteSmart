#ifndef WRITESMART_CLAUDE_PROVIDER_H
#define WRITESMART_CLAUDE_PROVIDER_H

#include "core/ai/AIProvider.h"

class ClaudeProvider : public AIProvider {
public:
    ClaudeProvider();

    QString providerId() const override;
    QString providerName() const override;
    QStringList supportedModels() const override;

    bool isConfigured() const override;
    bool configure(const QJsonObject& config) override;
    QJsonObject configuration() const override;

    void chat(const ChatRequest& request, ChatCallback callback) override;
    void chatStream(const ChatRequest& request, StreamCallback callback) override;
    void cancel() override;

private:
    QJsonObject buildRequestBody(const ChatRequest& request) const;
    QString m_apiKey;
    QString m_baseUrl = "https://api.anthropic.com";
    QString m_model = "claude-3-5-sonnet-20241022";
    double m_temperature = 0.7;
    int m_maxTokens = 4096;
    bool m_configured = false;
};

#endif
