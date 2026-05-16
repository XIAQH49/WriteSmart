#include "ui/panels/ChatPanel.h"
#include "ui/widgets/ChatBubble.h"
#include "core/ai/AISession.h"
#include "core/ai/providers/OpenAIProvider.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QLabel>
#include <QEvent>
#include <QKeyEvent>
#include <QTimer>

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

    auto* headerLayout = new QHBoxLayout();
    auto* titleLabel = new QLabel("AI 助手", this);
    QFont titleFont;
    titleFont.setBold(true);
    titleFont.setPointSize(13);
    titleLabel->setFont(titleFont);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    m_retryButton = new QPushButton("重试", this);
    m_retryButton->setFixedHeight(26);
    m_retryButton->setToolTip("重新发送上一条消息");
    m_retryButton->setVisible(false);
    headerLayout->addWidget(m_retryButton);

    m_newSessionButton = new QPushButton("新会话", this);
    m_newSessionButton->setFixedHeight(26);
    headerLayout->addWidget(m_newSessionButton);

    layout->addLayout(headerLayout);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_messageContainer = new QWidget(m_scrollArea);
    m_messageLayout = new QVBoxLayout(m_messageContainer);
    m_messageLayout->setAlignment(Qt::AlignTop);
    m_messageLayout->addStretch();

    m_scrollArea->setWidget(m_messageContainer);
    layout->addWidget(m_scrollArea, 1);

    auto* inputLayout = new QHBoxLayout();

    m_inputEdit = new QTextEdit(this);
    m_inputEdit->setPlaceholderText("输入消息，Ctrl+Enter 发送...");
    m_inputEdit->setFixedHeight(70);
    m_inputEdit->setAcceptRichText(false);
    m_inputEdit->installEventFilter(this);
    inputLayout->addWidget(m_inputEdit);

    m_stopButton = new QPushButton("停止", this);
    m_stopButton->setFixedSize(56, 70);
    m_stopButton->setVisible(false);
    m_stopButton->setStyleSheet(
        "QPushButton { background-color: #e64553; color: white; border-radius: 4px; }"
        "QPushButton:hover { background-color: #d20f39; }");
    inputLayout->addWidget(m_stopButton);

    m_sendButton = new QPushButton("发送", this);
    m_sendButton->setFixedSize(56, 70);
    inputLayout->addWidget(m_sendButton);

    layout->addLayout(inputLayout);

    m_provider = std::make_shared<OpenAIProvider>();
}

void ChatPanel::setupConnections()
{
    connect(m_sendButton, &QPushButton::clicked, this, &ChatPanel::onSendMessage);
    connect(m_stopButton, &QPushButton::clicked, this, &ChatPanel::onStopStream);
    connect(m_retryButton, &QPushButton::clicked, this, &ChatPanel::onRetry);
    connect(m_newSessionButton, &QPushButton::clicked, this, &ChatPanel::onNewSession);
}

void ChatPanel::setAIProvider(AIProviderPtr provider)
{
    m_provider = std::move(provider);
    m_session.reset();
}

AIProviderPtr ChatPanel::provider() const { return m_provider; }

void ChatPanel::injectContext(const QString& context)
{
    m_injectedContext = context;
    if (m_session) m_session->injectContext(context);
}

bool ChatPanel::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_inputEdit && event->type() == QEvent::KeyPress) {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return &&
            (keyEvent->modifiers() & Qt::ControlModifier)) {
            onSendMessage();
            return true;
        }
        if (keyEvent->key() == Qt::Key_Return &&
            keyEvent->modifiers() == Qt::NoModifier && m_streaming) {
            return true;  // block plain Enter during streaming
        }
    }
    return QWidget::eventFilter(obj, event);
}

void ChatPanel::onSendMessage()
{
    QString text = m_inputEdit->toPlainText().trimmed();
    if (text.isEmpty()) return;

    if (m_streaming) return;

    if (!m_provider || !m_provider->isConfigured()) {
        addMessage("system", "AI 尚未配置。请在菜单栏 设置 → AI API 中配置 API Key 和端点地址。\n\n支持 OpenAI、Claude 及兼容的第三方 API。");
        return;
    }

    if (!m_session) {
        m_session = std::make_unique<AISession>(m_provider);
        m_session->setSystemPrompt(
            "你是专业文学创作助手。根据用户需求提供：\n"
            "1. 文字润色 — 改进表达、修正语法\n"
            "2. 创意灵感 — 情节建议、角色塑造\n"
            "3. 逻辑检查 — 发现前后矛盾\n"
            "回答简洁有力，直击要点。如果用户给了上下文，优先基于上下文回答。");
    }

    m_lastUserMessage = text;
    addMessage("user", text);
    m_inputEdit->clear();
    addMessage("assistant", "", true);
    setStreamingState(true);

    m_session->sendStream(text,
        [this](const QString& delta, bool done, const QString& error) {
            if (!error.isEmpty()) {
                QMetaObject::invokeMethod(this, [this, error]() {
                    onStreamError(error);
                }, Qt::QueuedConnection);
                return;
            }
            QMetaObject::invokeMethod(this, [this, delta, done]() {
                if (!done) onStreamToken(delta);
                else onStreamComplete();
            }, Qt::QueuedConnection);
        });
}

void ChatPanel::onStopStream()
{
    if (m_session) m_session->cancel();
    setStreamingState(false);
    ChatBubble* bubble = lastAssistantBubble();
    if (bubble) {
        if (bubble->content().isEmpty()) bubble->setContent("(已停止)");
        bubble->markAsComplete();
    }
}

void ChatPanel::onRetry()
{
    if (m_lastUserMessage.isEmpty()) return;
    // 删除最后一条 assistant 消息
    QLayoutItem* lastItem = m_messageLayout->itemAt(m_messageLayout->count() - 2);
    if (lastItem && lastItem->widget()) {
        lastItem->widget()->deleteLater();
        delete lastItem;
    }
    if (!m_messages.isEmpty() && m_messages.last().role == "assistant") {
        m_messages.removeLast();
    }
    QString msg = m_lastUserMessage;
    m_lastUserMessage.clear();
    m_inputEdit->setPlainText(msg);
    onSendMessage();
}

void ChatPanel::onStreamToken(const QString& token)
{
    ChatBubble* bubble = lastAssistantBubble();
    if (bubble) {
        bubble->appendContent(token);
        scrollToBottom();
    }
}

void ChatPanel::onStreamComplete()
{
    setStreamingState(false);
    ChatBubble* bubble = lastAssistantBubble();
    if (bubble) bubble->markAsComplete();
    m_retryButton->setVisible(true);
}

void ChatPanel::onStreamError(const QString& error)
{
    setStreamingState(false);
    ChatBubble* bubble = lastAssistantBubble();
    if (bubble) {
        bubble->setContent("请求失败: " + error);
        bubble->markAsComplete();
    }
    m_retryButton->setVisible(true);
}

void ChatPanel::onNewSession()
{
    if (m_session) m_session->cancel();
    m_session.reset();
    m_lastUserMessage.clear();
    m_streaming = false;
    m_retryButton->setVisible(false);

    QLayoutItem* item;
    while ((item = m_messageLayout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    m_messageLayout->addStretch();
    m_messages.clear();

    addMessage("system", "新会话已开始。选中编辑区文本 → 自动注入为对话上下文。");
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
    scrollToBottom();
}

ChatBubble* ChatPanel::lastAssistantBubble() const
{
    for (int i = m_messageLayout->count() - 1; i >= 0; --i) {
        QLayoutItem* li = m_messageLayout->itemAt(i);
        if (!li || !li->widget()) continue;
        ChatBubble* bubble = qobject_cast<ChatBubble*>(li->widget());
        if (bubble && bubble->isStreaming()) return bubble;
    }
    return nullptr;
}

void ChatPanel::setStreamingState(bool streaming)
{
    m_streaming = streaming;
    m_sendButton->setVisible(!streaming);
    m_stopButton->setVisible(streaming);
    m_retryButton->setVisible(!streaming && !m_lastUserMessage.isEmpty());
    m_inputEdit->setReadOnly(streaming);
    if (streaming) {
        m_inputEdit->setPlaceholderText("AI 正在回复中...");
    } else {
        m_inputEdit->setPlaceholderText("输入消息，Ctrl+Enter 发送...");
    }
}

void ChatPanel::scrollToBottom()
{
    QScrollBar* bar = m_scrollArea->verticalScrollBar();
    QTimer::singleShot(0, this, [bar]() { bar->setValue(bar->maximum()); });
}
