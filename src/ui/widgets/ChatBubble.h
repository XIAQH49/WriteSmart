#ifndef WRITESMART_CHAT_BUBBLE_H
#define WRITESMART_CHAT_BUBBLE_H

#include <QFrame>
#include <QLabel>

class ChatBubble : public QFrame {
    Q_OBJECT

public:
    explicit ChatBubble(const QString& role, const QString& content, QWidget* parent = nullptr);

    void appendContent(const QString& delta);
    void setContent(const QString& content);
    QString content() const;

    void markAsComplete();

    bool isStreaming() const { return m_streaming; }

private:
    void setupUI(const QString& role);

    QLabel* m_contentLabel = nullptr;
    QLabel* m_roleLabel = nullptr;
    QString m_role;
    bool m_streaming = false;
};

#endif // WRITESMART_CHAT_BUBBLE_H
