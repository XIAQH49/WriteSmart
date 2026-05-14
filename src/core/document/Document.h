#ifndef WRITESMART_DOCUMENT_H
#define WRITESMART_DOCUMENT_H

#include <QString>
#include <QDateTime>
#include <QList>
#include <QVector>
#include <QJsonObject>
#include <memory>

struct Paragraph {
    QString id;
    QString text;
    int styleLevel = 0;       // 0=正文, 1-6=标题级别
    QDateTime created;
    QDateTime modified;
};

struct Chapter {
    QString id;
    QString title;
    QList<Paragraph> paragraphs;
    int wordCount = 0;
    QDateTime created;
    QDateTime modified;
};

struct Character {
    QString id;
    QString name;
    QString description;
    QStringList aliases;
    QJsonObject attributes;   // 自定义属性
};

struct OutlineNode {
    QString id;
    QString title;
    int level = 0;
    QString linkedChapterId;
    QList<std::shared_ptr<OutlineNode>> children;
};

class Document {
public:
    Document();
    explicit Document(const QString& filePath);
    ~Document();

    QString filePath() const;
    void setFilePath(const QString& path);

    QString title() const;
    void setTitle(const QString& title);

    QString author() const;
    void setAuthor(const QString& author);

    QDateTime created() const;
    QDateTime modified() const;

    // 章节管理
    QList<Chapter>& chapters();
    const QList<Chapter>& chapters() const;
    void addChapter(const Chapter& chapter);
    void removeChapter(const QString& chapterId);
    Chapter* findChapter(const QString& chapterId);

    // 人物管理
    QList<Character>& characters();
    const QList<Character>& characters() const;
    void addCharacter(const Character& character);

    // 大纲管理
    std::shared_ptr<OutlineNode> outlineRoot();
    void setOutlineRoot(std::shared_ptr<OutlineNode> root);

    // 统计
    int totalWordCount() const;
    int chapterCount() const;

    // 序列化
    bool save(const QString& filePath = QString());
    bool load(const QString& filePath);
    QString toMarkdown() const;
    static std::unique_ptr<Document> fromMarkdown(const QString& markdown);

    // 脏标记
    bool isModified() const;
    void markClean();

private:
    QString m_filePath;
    QString m_title;
    QString m_author;
    QDateTime m_created;
    QDateTime m_modified;
    QList<Chapter> m_chapters;
    QList<Character> m_characters;
    std::shared_ptr<OutlineNode> m_outlineRoot;
    bool m_dirty = false;
};

using DocumentPtr = std::shared_ptr<Document>;

#endif // WRITESMART_DOCUMENT_H
