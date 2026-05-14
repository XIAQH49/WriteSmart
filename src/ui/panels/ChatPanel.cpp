#include "ui/panels/ChatPanel.h"
#include "ui/widgets/ChatBubble.h"
#include "core/ai/AISession.h"
#include "core/ai/providers/OpenAIProvider.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QLabel>

ChatPanel::ChatPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    setupConnections();
    setMinimumWidth(280);
}

ChatPanel::~ChatPanel() = default;

void ChatPanel::setupUI()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);

    // 标题栏
    auto* headerLayout = new QHBoxLayout();
    auto* titleLabel = new QLabel("AI 助手", this);
    QFont titleFont;
    titleFont.setBold(true);
    titleFont.setPointSize(13);
    titleLabel->setFont(titleFont);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    m_newSessionButton = new QPushButton("新会话", this);
    m_newSessionButton->setFixedHeight(28);
    headerLayout->addWidget(m_newSessionButton);

    layout->addLayout(headerLayout);

    // 消息区域
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_messageContainer = new QWidget(m_scrollArea);
    m_messageLayout = new QVBoxLayout(m_messageContainer);
    m_messageLayout->setAlignment(Qt::AlignTop);
    m_messageLayout->addStretch();

    m_scrollArea->setWidget(m_messageContainer);
    layout->addWidget(m_scrollArea, 1);

    // 输入区
    auto* inputLayout = new QHBoxLayout();

    m_inputEdit = new QTextEdit(this);
    m_inputEdit->setPlaceholderText("输入消息，Ctrl+Enter 发送");
    m_inputEdit->setFixedHeight(70);
    m_inputEdit->setAcceptRichText(false);
    inputLayout->addWidget(m_inputEdit);

    m_sendButton = new QPushButton("发送", this);
    m_sendButton->setFixedSize(60, 70);
    inputLayout->addWidget(m_sendButton);

    layout->addLayout(inputLayout);

    // 默认Provider
    m_provider = std::make_shared<OpenAIProvider>();
}

void ChatPanel::setupConnections()
{
    connect(m_sendButton, &QPushButton::clicked, this, &ChatPanel::onSendMessage);
    connect(m_newSessionButton, &QPushButton::clicked, this, &ChatPanel::onNewSession);

    connect(m_inputEdit, &QTextEdit::textChanged, this, [this]() {
        // Ctrl+Enter 快捷发送
    });

    // 监听 Ctrl+Enter
    m_inputEdit->installEventFilter(this);
}

void ChatPanel::setAIProvider(AIProviderPtr provider)
{
    m_provider = std::move(provider);
}

void ChatPanel::injectContext(const QString& context)
{
    m_injectedContext = context;
    if (m_session) {
        m_session->injectContext(context);
    }
}

void ChatPanel::onSendMessage()
{
    QString text = m_inputEdit->toPlainText().trimmed();
    if (text.isEmpty()) return;

    if (!m_provider || !m_provider->isConfigured()) {
        addMessage("system", "请先配置 AI API，在设置中填入 API Key 和端点地址。");
        return;
    }

    if (!m_session) {
        m_session = std::make_unique<AISession>(m_provider);
        m_session->setSystemPrompt("你是一位专业的文学创作助手。帮助用户润色文字、提供创意灵感、检查逻辑连贯性。回答简洁有力，直接切中要点。");
    }

    addMessage("user", text);
    m_inputEdit->clear();

    addMessage("assistant", "", true);

    m_session->sendStream(text,
        [this](const QString& delta, bool done, const QString& error) {
            if (!error.isEmpty()) {
                onStreamError(error);
                return;
            }
            QMetaObject::invokeMethod(this, [this, delta, done]() {
                if (!done) {
                    onStreamToken(delta);
                } else {
                    onStreamComplete();
                }
            }, Qt::QueuedConnection);
        });
}

void ChatPanel::onStreamToken(const QString& token)
{
    ChatBubble* bubble = lastAssistantBubble();
    if (bubble) {
        bubble->appendContent(token);
        // 自动滚动到底部
        QScrollBar* bar = m_scrollArea->verticalScrollBar();
        bar->setValue(bar->maximum());
    }
}

void ChatPanel::onStreamComplete()
{
    ChatBubble* bubble = lastAssistantBubble();
    if (bubble) {
        bubble->markAsComplete();
    }
}

void ChatPanel::onStreamError(const QString& error)
{
    ChatBubble* bubble = lastAssistantBubble();
    if (bubble) {
        bubble->setContent("错误: " + error);
        bubble->markAsComplete();
    }
}

void ChatPanel::onNewSession()
{
    if (m_session) {
        m_session->clearHistory();
    }
    m_session.reset();

    // 清除UI
    QLayoutItem* item;
    while ((item = m_messageLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    m_messageLayout->addStretch();
    m_messages.clear();

    addMessage("system", "新会话已开始。选中编辑区文本后，会自动注入为对话上下文。");
}

void ChatPanel::onClearSession()
{
    onNewSession();
}

void ChatPanel::addMessage(const QString& role, const QString& content, bool streaming)
{
    ChatMessageData data;
    data.role = role;
    data.content = content;
    data.streaming = streaming;
    m_messages.append(data);

    ChatBubble* bubble = new ChatBubble(role, content, m_messageContainer);
    int insertPos = m_messageLayout->count() - 1;
    m_messageLayout->insertWidget(insertPos, bubble);
}

ChatBubble* ChatPanel::lastAssistantBubble() const
{
    for (int i = m_messageLayout->count() - 1; i >= 0; --i) {
        QWidget* w = m_messageLayout->itemAt(i)->widget();
        ChatBubble* bubble = qobject_cast<ChatBubble*>(w);
        if (bubble && bubble->isStreaming()) {
            return bubble;
        }
    }
    return nullptr;
}
