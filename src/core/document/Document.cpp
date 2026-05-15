#include "core/document/Document.h"
#include "utils/JsonHelper.h"
#include "utils/StringUtils.h"

Document::Document()
{
    m_fileId = StringUtils::generateId("doc");
    m_created = QDateTime::currentDateTime();
    m_modified = m_created;
    m_outlineRoot = std::make_shared<OutlineNode>();
    m_outlineRoot->id = StringUtils::generateId("outline");
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
    for (const auto& ch : m_chapters) total += ch.wordCount;
    return total;
}

int Document::chapterCount() const { return m_chapters.size(); }

// ============================================================
// JSON 序列化
// ============================================================
static QJsonObject chapterToJson(const Chapter& ch)
{
    QJsonArray paras;
    for (const auto& p : ch.paragraphs) {
        paras.append(QJsonObject{
            {"id", p.id}, {"text", p.text}, {"styleLevel", p.styleLevel}
        });
    }
    return QJsonObject{
        {"id", ch.id}, {"title", ch.title}, {"paragraphs", paras},
        {"wordCount", ch.wordCount}
    };
}

static Chapter chapterFromJson(const QJsonObject& obj)
{
    Chapter ch;
    ch.id = obj["id"].toString(StringUtils::generateId("ch"));
    ch.title = obj["title"].toString();
    for (const auto& pv : obj["paragraphs"].toArray()) {
        QJsonObject po = pv.toObject();
        Paragraph p;
        p.id = po["id"].toString(StringUtils::generateId("p"));
        p.text = po["text"].toString();
        p.styleLevel = po["styleLevel"].toInt(0);
        ch.paragraphs.append(p);
    }
    ch.wordCount = obj["wordCount"].toInt();
    return ch;
}

static QJsonObject characterToJson(const Character& ch)
{
    return QJsonObject{
        {"id", ch.id}, {"name", ch.name}, {"description", ch.description}
    };
}

static Character characterFromJson(const QJsonObject& obj)
{
    Character ch;
    ch.id = obj["id"].toString(StringUtils::generateId("char"));
    ch.name = obj["name"].toString();
    ch.description = obj["description"].toString();
    return ch;
}

static QJsonObject outlineToJson(const OutlineNode* node)
{
    QJsonArray children;
    for (const auto& c : node->children) {
        children.append(outlineToJson(c.get()));
    }
    return QJsonObject{
        {"id", node->id}, {"title", node->title}, {"level", node->level},
        {"linkedChapterId", node->linkedChapterId}, {"children", children}
    };
}

static std::shared_ptr<OutlineNode> outlineFromJson(const QJsonObject& obj)
{
    auto node = std::make_shared<OutlineNode>();
    node->id = obj["id"].toString(StringUtils::generateId("ol"));
    node->title = obj["title"].toString();
    node->level = obj["level"].toInt(0);
    node->linkedChapterId = obj["linkedChapterId"].toString();
    for (const auto& cv : obj["children"].toArray()) {
        node->children.append(outlineFromJson(cv.toObject()));
    }
    return node;
}

QJsonObject Document::toJson() const
{
    QJsonArray chs;
    for (const auto& ch : m_chapters) chs.append(chapterToJson(ch));

    QJsonArray chars;
    for (const auto& ch : m_characters) chars.append(characterToJson(ch));

    return QJsonObject{
        {"fileId", m_fileId},
        {"title", m_title},
        {"author", m_author},
        {"chapters", chs},
        {"characters", chars},
        {"outline", m_outlineRoot ? outlineToJson(m_outlineRoot.get()) : QJsonObject()}
    };
}

bool Document::save(const QString& filePath)
{
    QString path = filePath.isEmpty() ? m_filePath : filePath;
    if (path.isEmpty()) return false;

    m_modified = QDateTime::currentDateTime();
    QJsonObject obj = toJson();
    obj["lastModified"] = m_modified.toString(Qt::ISODate);
    obj["created"] = m_created.toString(Qt::ISODate);

    if (JsonHelper::toFile(path, obj)) {
        m_filePath = path;
        m_dirty = false;
        return true;
    }
    return false;
}

bool Document::load(const QString& filePath)
{
    QJsonObject obj = JsonHelper::fromFile(filePath);
    if (obj.isEmpty()) return false;

    m_fileId = obj["fileId"].toString();
    m_title = obj["title"].toString();
    m_author = obj["author"].toString();
    m_filePath = filePath;
    m_created = QDateTime::fromString(obj["created"].toString(), Qt::ISODate);
    m_modified = QDateTime::fromString(obj["lastModified"].toString(), Qt::ISODate);

    m_chapters.clear();
    for (const auto& cv : obj["chapters"].toArray()) {
        m_chapters.append(chapterFromJson(cv.toObject()));
    }

    m_characters.clear();
    for (const auto& cv : obj["characters"].toArray()) {
        m_characters.append(characterFromJson(cv.toObject()));
    }

    if (obj.contains("outline")) {
        m_outlineRoot = outlineFromJson(obj["outline"].toObject());
    }

    m_dirty = false;
    return true;
}

QString Document::toMarkdown() const
{
    QString md;
    if (!m_title.isEmpty()) md += "# " + m_title + "\n\n";
    for (const auto& ch : m_chapters) {
        if (!ch.title.isEmpty()) md += "\n## " + ch.title + "\n\n";
        for (const auto& p : ch.paragraphs) {
            md += p.text + "\n\n";
        }
    }
    return md;
}

std::unique_ptr<Document> Document::fromMarkdown(const QString& /*markdown*/)
{
    return std::make_unique<Document>();
}

bool Document::isModified() const { return m_dirty; }
void Document::markClean() { m_dirty = false; }
