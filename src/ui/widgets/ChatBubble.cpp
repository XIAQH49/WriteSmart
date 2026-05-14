#include "ui/widgets/ChatBubble.h"
#include <QVBoxLayout>

ChatBubble::ChatBubble(const QString& role, const QString& content, QWidget* parent)
    : QFrame(parent)
    , m_role(role)
{
    setupUI(role);
    setContent(content);
}

void ChatBubble::setupUI(const QString& role)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 8, 12, 8);

    m_roleLabel = new QLabel(this);
    QFont roleFont;
    roleFont.setBold(true);
    roleFont.setPointSize(11);
    m_roleLabel->setFont(roleFont);

    if (role == "user") {
        m_roleLabel->setText("你");
        setStyleSheet("ChatBubble { background-color: #313244; border-radius: 10px; margin-right: 20px; }");
    } else if (role == "assistant") {
        m_roleLabel->setText("AI");
        setStyleSheet("ChatBubble { background-color: #45475a; border-radius: 10px; margin-left: 20px; }");
    } else {
        m_roleLabel->setText(role);
        setStyleSheet("ChatBubble { background-color: #1e1e2e; border-radius: 10px; }");
    }
    layout->addWidget(m_roleLabel);

    m_contentLabel = new QLabel(this);
    m_contentLabel->setWordWrap(true);
    m_contentLabel->setTextFormat(Qt::PlainText);
    QFont contentFont;
    contentFont.setPointSize(12);
    m_contentLabel->setFont(contentFont);
    layout->addWidget(m_contentLabel);
}

void ChatBubble::appendContent(const QString& delta)
{
    m_contentLabel->setText(m_contentLabel->text() + delta);
    m_streaming = true;
}

void ChatBubble::setContent(const QString& content)
{
    m_contentLabel->setText(content);
}

QString ChatBubble::content() const
{
    return m_contentLabel->text();
}

void ChatBubble::markAsComplete()
{
    m_streaming = false;
}
