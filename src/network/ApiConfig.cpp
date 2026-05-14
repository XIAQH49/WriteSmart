#include "network/ApiConfig.h"
#include "utils/JsonHelper.h"

bool ApiConfig::save(const QString& path)
{
    return JsonHelper::toFile(path, toJson());
}

bool ApiConfig::load(const QString& path)
{
    QJsonObject obj = JsonHelper::fromFile(path);
    if (obj.isEmpty()) return false;
    *this = fromJson(obj);
    return true;
}

ApiEndpoint* ApiConfig::findEndpoint(const QString& baseUrl)
{
    for (auto& ep : endpoints) {
        if (ep.baseUrl == baseUrl) return &ep;
    }
    return nullptr;
}

QJsonObject ApiConfig::toJson() const
{
    QJsonObject obj;
    obj["defaultProvider"] = defaultProvider;
    QJsonArray eps;
    for (const auto& ep : endpoints) {
        QJsonObject e;
        e["baseUrl"] = ep.baseUrl;
        e["chatPath"] = ep.chatPath;
        e["modelsPath"] = ep.modelsPath;
        e["apiKey"] = ep.apiKey;
        e["model"] = ep.model;
        e["headers"] = ep.headers;
        e["requestTemplate"] = ep.requestTemplate;
        e["responseContentPath"] = ep.responseContentPath;
        e["temperature"] = ep.temperature;
        e["maxTokens"] = ep.maxTokens;
        e["timeoutSecs"] = ep.timeoutSecs;
        eps.append(e);
    }
    obj["endpoints"] = eps;
    return obj;
}

ApiConfig ApiConfig::fromJson(const QJsonObject& json)
{
    ApiConfig config;
    config.defaultProvider = json["defaultProvider"].toString();
    for (const auto& val : json["endpoints"].toArray()) {
        QJsonObject e = val.toObject();
        ApiEndpoint ep;
        ep.baseUrl = e["baseUrl"].toString();
        ep.chatPath = e["chatPath"].toString("/v1/chat/completions");
        ep.modelsPath = e["modelsPath"].toString("/v1/models");
        ep.apiKey = e["apiKey"].toString();
        ep.model = e["model"].toString();
        ep.headers = e["headers"].toObject();
        ep.requestTemplate = e["requestTemplate"].toObject();
        ep.responseContentPath = e["responseContentPath"].toString("choices[0].message.content");
        ep.temperature = e["temperature"].toDouble(0.7);
        ep.maxTokens = e["maxTokens"].toInt(4096);
        ep.timeoutSecs = e["timeoutSecs"].toInt(30);
        config.endpoints.append(ep);
    }
    return config;
}
