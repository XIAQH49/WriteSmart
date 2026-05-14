#ifndef WRITESMART_API_CONFIG_H
#define WRITESMART_API_CONFIG_H

#include <QString>
#include <QJsonObject>
#include <QJsonArray>

struct ApiEndpoint {
    QString baseUrl;
    QString chatPath = "/v1/chat/completions";
    QString modelsPath = "/v1/models";
    QString apiKey;
    QString model;
    QJsonObject headers;            // 自定义 HTTP Headers

    // 请求模板 (支持变量: {model}, {messages}, {temperature}, {max_tokens})
    QJsonObject requestTemplate;
    // 响应 JSON 路径 (如 "choices[0].message.content")
    QString responseContentPath = "choices[0].message.content";

    double temperature = 0.7;
    int maxTokens = 4096;
    int timeoutSecs = 30;
};

struct ApiConfig {
    QString defaultProvider;
    QList<ApiEndpoint> endpoints;

    bool save(const QString& path);
    bool load(const QString& path);

    ApiEndpoint* findEndpoint(const QString& baseUrl);
    QJsonObject toJson() const;
    static ApiConfig fromJson(const QJsonObject& json);
};

#endif // WRITESMART_API_CONFIG_H
