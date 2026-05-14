#include "core/document/Document.h"

Document::Document()
{
    m_created = QDateTime::currentDateTime();
    m_modified = m_created;
    m_outlineRoot = std::make_shared<OutlineNode>();
    m_outlineRoot->title = "Root";
}

Document::Document(const QString& filePath)
    : Document()
{
    m_filePath = filePath;
}

Document::~Document() = default;

QString Document::filePath() const { return m_filePath; }
void Document::setFilePath(const QString& path) { m_filePath = path; }

QString Document::title() const { return m_title; }
void Document::setTitle(const QString& title) { m_title = title; m_dirty = true; }

QString Document::author() const { return m_author; }
void Document::setAuthor(const QString& author) { m_author = author; }

QDateTime Document::created() const { return m_created; }
QDateTime Document::modified() const { return m_modified; }

QList<Chapter>& Document::chapters() { return m_chapters; }
const QList<Chapter>& Document::chapters() const { return m_chapters; }

void Document::addChapter(const Chapter& chapter)
{
    m_chapters.append(chapter);
    m_dirty = true;
}

void Document::removeChapter(const QString& chapterId)
{
    m_chapters.erase(std::remove_if(m_chapters.begin(), m_chapters.end(),
        [&](const Chapter& c) { return c.id == chapterId; }), m_chapters.end());
    m_dirty = true;
}

Chapter* Document::findChapter(const QString& chapterId)
{
    for (auto& c : m_chapters) {
        if (c.id == chapterId) return &c;
    }
    return nullptr;
}

QList<Character>& Document::characters() { return m_characters; }
const QList<Character>& Document::characters() const { return m_characters; }

void Document::addCharacter(const Character& character)
{
    m_characters.append(character);
    m_dirty = true;
}

std::shared_ptr<OutlineNode> Document::outlineRoot() { return m_outlineRoot; }
void Document::setOutlineRoot(std::shared_ptr<OutlineNode> root)
{
    m_outlineRoot = std::move(root);
    m_dirty = true;
}

int Document::totalWordCount() const
{
    int total = 0;
    for (const auto& ch : m_chapters) {
        total += ch.wordCount;
    }
    return total;
}

int Document::chapterCount() const { return m_chapters.size(); }

bool Document::save(const QString& filePath)
{
    QString path = filePath.isEmpty() ? m_filePath : filePath;
    if (path.isEmpty()) return false;
    m_dirty = false;
    return true;
}

bool Document::load(const QString& filePath)
{
    m_filePath = filePath;
    m_dirty = false;
    return true;
}

QString Document::toMarkdown() const
{
    return {};
}

std::unique_ptr<Document> Document::fromMarkdown(const QString& /*markdown*/)
{
    auto doc = std::make_unique<Document>();
    return doc;
}

bool Document::isModified() const { return m_dirty; }
void Document::markClean() { m_dirty = false; }
