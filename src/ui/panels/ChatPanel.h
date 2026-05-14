#ifndef WRITESMART_CHAT_PANEL_H
#define WRITESMART_CHAT_PANEL_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QList>
#include <memory>

#include "core/ai/AIProvider.h"

class AISession;
class ChatBubble;

struct ChatMessageData {
    QString role;       // "user" | "assistant" | "system"
    QString content;
    bool streaming = false;
};

class ChatPanel : public QWidget {
    Q_OBJECT

public:
    explicit ChatPanel(QWidget* parent = nullptr);
    ~ChatPanel() override;

    void setAIProvider(AIProviderPtr provider);
    void injectContext(const QString& context);

signals:
    void messageSent(const QString& message);

private slots:
    void onSendMessage();
    void onStreamToken(const QString& token);
    void onStreamComplete();
    void onStreamError(const QString& error);
    void onNewSession();
    void onClearSession();

private:
    void setupUI();
    void setupConnections();
    void addMessage(const QString& role, const QString& content, bool streaming = false);
    ChatBubble* lastAssistantBubble() const;

    QScrollArea* m_scrollArea = nullptr;
    QWidget* m_messageContainer = nullptr;
    QVBoxLayout* m_messageLayout = nullptr;

    QTextEdit* m_inputEdit = nullptr;
    QPushButton* m_sendButton = nullptr;
    QPushButton* m_newSessionButton = nullptr;

    AIProviderPtr m_provider;
    std::unique_ptr<AISession> m_session;
    QList<ChatMessageData> m_messages;
    QString m_injectedContext;
};

#endif // WRITESMART_CHAT_PANEL_H
