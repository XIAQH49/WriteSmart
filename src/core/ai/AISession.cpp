#include "core/ai/AISession.h"
#include "core/ai/PromptTemplate.h"

AISession::AISession(AIProviderPtr provider)
    : m_provider(std::move(provider))
    , m_model(m_provider->supportedModels().isEmpty() ? QString() : m_provider->supportedModels().first())
{
}

AISession::~AISession() = default;

void AISession::setSystemPrompt(const QString& prompt) { m_systemPrompt = prompt; }

void AISession::setSystemPromptTemplate(std::shared_ptr<PromptTemplate> tmpl)
{
    if (tmpl) {
        m_systemPrompt = tmpl->systemPrompt();
    }
}

void AISession::setModel(const QString& model) { m_model = model; }
void AISession::setTemperature(double temp) { m_temperature = temp; }
void AISession::setMaxTokens(int tokens) { m_maxTokens = tokens; }

void AISession::injectContext(const QString& context) { m_injectedContext = context; }
void AISession::clearContext() { m_injectedContext.clear(); }

void AISession::send(const QString& userMessage, ChatCallback callback)
{
    ChatRequest req = buildRequest(userMessage);

    m_provider->chat(req, [this, callback](bool success, const ChatResponse& resp, const QString& error) {
        if (success) {
            ChatMessage msg;
            msg.role = "assistant";
            msg.content = resp.content;
            m_history.append(msg);
        }
        if (callback) callback(success, resp, error);
    });
}

void AISession::sendStream(const QString& userMessage, StreamCallback callback)
{
    ChatRequest req = buildRequest(userMessage);

    QString fullContent;
    m_provider->chatStream(req, [this, callback, &fullContent](const QString& delta, bool done, const QString& error) {
        fullContent += delta;
        if (callback) callback(delta, done, error);
        if (done && error.isEmpty()) {
            ChatMessage msg;
            msg.role = "assistant";
            msg.content = fullContent;
            m_history.append(msg);
        }
    });
}

void AISession::cancel()
{
    if (m_provider) {
        m_provider->cancel();
    }
}

QList<ChatMessage> AISession::history() const { return m_history; }

int AISession::estimatedTokens() const
{
    int total = 0;
    if (!m_systemPrompt.isEmpty()) total += m_systemPrompt.length() / 4;
    for (const auto& msg : m_history) {
        total += msg.content.length() / 4;
    }
    return total;
}

void AISession::clearHistory() { m_history.clear(); }

ChatRequest AISession::buildRequest(const QString& userMessage) const
{
    ChatRequest req;
    req.model = m_model;
    req.temperature = m_temperature;
    req.maxTokens = m_maxTokens;

    if (!m_systemPrompt.isEmpty()) {
        ChatMessage sys;
        sys.role = "system";
        sys.content = m_systemPrompt;
        req.messages.append(sys);
    }

    for (const auto& msg : m_history) {
        req.messages.append(msg);
    }

    if (!m_injectedContext.isEmpty()) {
        ChatMessage ctxMsg;
        ctxMsg.role = "system";
        ctxMsg.content = "当前编辑上下文：\n" + m_injectedContext;
        req.messages.append(ctxMsg);
    }

    ChatMessage user;
    user.role = "user";
    user.content = userMessage;
    req.messages.append(user);

    return req;
}

void AISession::trimHistory()
{
    while (estimatedTokens() > m_maxHistoryTokens && !m_history.isEmpty()) {
        m_history.removeFirst();
    }
}
