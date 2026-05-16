#include "ui/panels/EditorPanel.h"
#include "core/document/Document.h"
#include "utils/Config.h"
#include "utils/StringUtils.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QFont>
#include <QTextBlock>
#include <QTextCursor>

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
    m_editor->setPlaceholderText("开始你的创作之旅...\n\n提示：\n· 选中文本后，右侧AI助手自动获取上下文\n· Ctrl+S 保存文档\n· 双击大纲节点可跳转到对应章节");

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
    m_currentChapterId.clear();
    m_lastSavedRevision = 0;
    syncDocumentToEditor();
}

void EditorPanel::syncDocumentToEditor()
{
    if (!m_document) {
        m_editor->clear();
        return;
    }

    // block signals to avoid triggering onTextChanged during sync
    m_editor->blockSignals(true);
    buildEditorTextFromDocument();
    m_editor->document()->setModified(false);
    m_editor->blockSignals(false);
}

void EditorPanel::buildEditorTextFromDocument()
{
    QString fullText;
    for (const auto& ch : m_document->chapters()) {
        if (!fullText.isEmpty()) fullText += "\n\n";
        if (!ch.title.isEmpty()) {
            fullText += "## " + ch.title + "\n\n";
        }
        for (const auto& p : ch.paragraphs) {
            if (p.styleLevel > 0) {
                fullText += QString(p.styleLevel, '#') + " " + p.text + "\n\n";
            } else {
                fullText += p.text + "\n\n";
            }
        }
    }
    m_editor->setPlainText(fullText.trimmed());
}

void EditorPanel::syncEditsToDocument()
{
    if (!m_document) return;

    // 将编辑器纯文本回写为 Chapter/Paragraph 结构
    QString text = m_editor->toPlainText();
    m_document->chapters().clear();

    Chapter currentChapter;
    currentChapter.id = StringUtils::generateId("ch");
    currentChapter.title = m_document->title();
    QStringList lines = text.split('\n');

    Paragraph currentPara;
    currentPara.id = StringUtils::generateId("p");

    for (const QString& rawLine : lines) {
        QString line = rawLine;
        // 检测 ## 标题 → 新章节
        if (line.startsWith("## ") && line.length() > 3) {
            if (!currentChapter.paragraphs.isEmpty()) {
                m_document->addChapter(currentChapter);
            }
            currentChapter = Chapter{};
            currentChapter.id = StringUtils::generateId("ch");
            currentChapter.title = line.mid(3).trimmed();
            currentPara = Paragraph{};
            currentPara.id = StringUtils::generateId("p");
            continue;
        }
        // 检测 # 标题 → 段落标题
        if (line.startsWith("# ") && !line.startsWith("## ")) {
            if (!currentPara.text.isEmpty()) {
                currentChapter.paragraphs.append(currentPara);
            }
            currentPara = Paragraph{};
            currentPara.id = StringUtils::generateId("p");
            currentPara.styleLevel = 1;
            currentPara.text = line.mid(2);
            currentChapter.paragraphs.append(currentPara);
            int wc = StringUtils::totalWordCount(currentPara.text);
            currentChapter.wordCount += wc;
            currentPara = Paragraph{};
            currentPara.id = StringUtils::generateId("p");
            continue;
        }
        // 空行 → 段落分隔
        if (line.trimmed().isEmpty()) {
            if (!currentPara.text.isEmpty()) {
                currentChapter.paragraphs.append(currentPara);
                int wc = StringUtils::totalWordCount(currentPara.text);
                currentChapter.wordCount += wc;
                currentPara = Paragraph{};
                currentPara.id = StringUtils::generateId("p");
            }
            continue;
        }
        // 累积到当前段落
        if (!currentPara.text.isEmpty()) currentPara.text += '\n';
        currentPara.text += line;
    }
    // 刷最后一个段落和章节
    if (!currentPara.text.isEmpty()) {
        currentChapter.paragraphs.append(currentPara);
        currentChapter.wordCount += StringUtils::totalWordCount(currentPara.text);
    }
    if (!currentChapter.paragraphs.isEmpty()) {
        m_document->addChapter(currentChapter);
    }

    m_lastSavedRevision = m_editor->document()->revision();
}

void EditorPanel::navigateToChapter(const QString& chapterId)
{
    if (!m_document) return;
    m_currentChapterId = chapterId;
    int line = findChapterLine(chapterId);
    if (line >= 0) {
        scrollToLine(line);
    }
}

int EditorPanel::findChapterLine(const QString& chapterId) const
{
    Chapter* ch = m_document->findChapter(chapterId);
    if (!ch) return -1;

    // 在编辑器文本中搜索章节标题
    QString searchTitle = ch->title;
    if (searchTitle.isEmpty()) return 0;

    QString text = m_editor->toPlainText();
    int pos = text.indexOf(searchTitle);
    if (pos < 0) return 0;

    return text.left(pos).count('\n');
}

void EditorPanel::scrollToLine(int lineNumber)
{
    QTextDocument* doc = m_editor->document();
    if (lineNumber >= doc->blockCount()) lineNumber = doc->blockCount() - 1;
    QTextBlock block = doc->findBlockByNumber(lineNumber);
    if (!block.isValid()) return;

    QTextCursor cursor(block);
    m_editor->setTextCursor(cursor);
    m_editor->centerCursor();
    m_editor->setFocus();
}

QString EditorPanel::selectedText() const
{
    return m_editor->textCursor().selectedText();
}

QString EditorPanel::fullText() const
{
    return m_editor->toPlainText();
}

int EditorPanel::currentWordCount() const
{
    return StringUtils::totalWordCount(m_editor->toPlainText());
}

QString EditorPanel::currentChapterId() const
{
    return m_currentChapterId;
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
    syncEditsToDocument();
    emit documentModified();
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
    emit wordCountChanged(currentWordCount());
}
