#ifndef WRITESMART_CHAT_PANEL_H
#define WRITESMART_CHAT_PANEL_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QList>
#include <QKeyEvent>
#include <memory>

#include "core/ai/AIProvider.h"

class AISession;
class ChatBubble;

struct ChatMessageData {
    QString role;
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
    AIProviderPtr provider() const;

signals:
    void messageSent(const QString& message);
    void providerConfigureRequested();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onSendMessage();
    void onStopStream();
    void onRetry();
    void onStreamToken(const QString& token);
    void onStreamComplete();
    void onStreamError(const QString& error);
    void onNewSession();

private:
    void setupUI();
    void setupConnections();
    void addMessage(const QString& role, const QString& content, bool streaming = false);
    ChatBubble* lastAssistantBubble() const;
    void setStreamingState(bool streaming);
    void scrollToBottom();

    QScrollArea* m_scrollArea = nullptr;
    QWidget* m_messageContainer = nullptr;
    QVBoxLayout* m_messageLayout = nullptr;

    QTextEdit* m_inputEdit = nullptr;
    QPushButton* m_sendButton = nullptr;
    QPushButton* m_stopButton = nullptr;
    QPushButton* m_retryButton = nullptr;
    QPushButton* m_newSessionButton = nullptr;

    AIProviderPtr m_provider;
    std::unique_ptr<AISession> m_session;
    QList<ChatMessageData> m_messages;
    QString m_injectedContext;
    QString m_lastUserMessage;
    bool m_streaming = false;
};

#endif
