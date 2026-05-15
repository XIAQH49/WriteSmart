#ifndef WRITESMART_CUSTOM_PROVIDER_H
#define WRITESMART_CUSTOM_PROVIDER_H

#include "core/ai/AIProvider.h"
#include "network/ApiConfig.h"

class CustomProvider : public AIProvider {
public:
    CustomProvider();

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
    ApiEndpoint m_endpoint;
    bool m_configured = false;
};

#endif
