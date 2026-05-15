#include "ui/panels/EditorPanel.h"
#include "core/document/Document.h"
#include "utils/Config.h"
#include "utils/StringUtils.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QFont>

EditorPanel::EditorPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    setupConnections();
}

EditorPanel::~EditorPanel() = default;

void EditorPanel::setupUI()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_editor = new QPlainTextEdit(this);
    QFont font(Config::instance().fontFamily(), Config::instance().fontSize());
    m_editor->setFont(font);
    m_editor->setTabStopDistance(40);
    m_editor->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    m_editor->setPlaceholderText("开始你的创作之旅...");

    layout->addWidget(m_editor);

    m_autoSaveTimer = new QTimer(this);
    m_autoSaveTimer->setSingleShot(true);
    m_autoSaveTimer->setInterval(Config::instance().autoSaveIntervalMs());
}

void EditorPanel::setupConnections()
{
    connect(m_editor, &QPlainTextEdit::textChanged, this, &EditorPanel::onTextChanged);
    connect(m_editor, &QPlainTextEdit::selectionChanged, this, &EditorPanel::onSelectionChanged);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &EditorPanel::onAutoSave);
}

void EditorPanel::setDocument(std::shared_ptr<Document> doc)
{
    m_document = std::move(doc);
    QString fullText;
    for (const auto& ch : m_document->chapters()) {
        for (const auto& p : ch.paragraphs) {
            if (!fullText.isEmpty()) fullText += "\n";
            if (p.styleLevel > 0) {
                fullText += QString(p.styleLevel, '#') + " " + p.text;
            } else {
                fullText += p.text;
            }
        }
    }
    m_editor->setPlainText(fullText);
}

void EditorPanel::navigateToChapter(const QString& chapterId)
{
    if (!m_document) return;
    // TODO: 滚动到对应章节
    m_currentChapterId = chapterId;
}

QString EditorPanel::selectedText() const
{
    QTextCursor cursor = m_editor->textCursor();
    return cursor.selectedText();
}

int EditorPanel::currentWordCount() const
{
    return StringUtils::totalWordCount(m_editor->toPlainText());
}

void EditorPanel::onTextChanged()
{
    if (Config::instance().autoSave()) {
        m_autoSaveTimer->start();
    }
    updateWordCount();
    emit documentModified();
}

void EditorPanel::onAutoSave()
{
    if (m_document) {
        // 将编辑内容写回文档模型
        emit documentModified();
    }
}

void EditorPanel::onSelectionChanged()
{
    QString text = selectedText();
    if (!text.isEmpty()) {
        emit textSelected(text);
    }
}

void EditorPanel::updateWordCount()
{
    int count = currentWordCount();
    emit wordCountChanged(count);
}

QString EditorPanel::currentChapterId() const
{
    return m_currentChapterId;
}
