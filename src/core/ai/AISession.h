#ifndef WRITESMART_AI_SESSION_H
#define WRITESMART_AI_SESSION_H

#include <QString>
#include <QList>
#include <memory>
#include "AIProvider.h"

class PromptTemplate;

class AISession {
public:
    explicit AISession(AIProviderPtr provider);
    ~AISession();

    void setSystemPrompt(const QString& prompt);
    void setSystemPromptTemplate(std::shared_ptr<PromptTemplate> tmpl);
    void setModel(const QString& model);
    void setTemperature(double temp);
    void setMaxTokens(int tokens);

    void injectContext(const QString& context);
    void clearContext();

    void send(const QString& userMessage, ChatCallback callback);
    void sendStream(const QString& userMessage, StreamCallback callback);
    void cancel();

    QList<ChatMessage> history() const;
    int estimatedTokens() const;
    void clearHistory();

private:
    ChatRequest buildRequest(const QString& userMessage) const;
    void trimHistory();

    AIProviderPtr m_provider;
    QString m_systemPrompt;
    QString m_model;
    double m_temperature = 0.7;
    int m_maxTokens = 4096;
    QString m_injectedContext;
    QList<ChatMessage> m_history;
    int m_maxHistoryTokens = 8000;
    bool m_busy = false;
};

#endif // WRITESMART_AI_SESSION_H
