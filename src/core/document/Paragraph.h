#ifndef WRITESMART_PARAGRAPH_H
#define WRITESMART_PARAGRAPH_H

#include <QString>
#include <QDateTime>

struct Paragraph {
    QString id;
    QString text;
    int styleLevel = 0;
    QDateTime created;
    QDateTime modified;

    bool isEmpty() const { return text.isEmpty(); }
    int wordCount() const;
};

namespace ParagraphUtils {
    int estimateWordCount(const QString& text);
}

#endif // WRITESMART_PARAGRAPH_H
