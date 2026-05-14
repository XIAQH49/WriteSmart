#include "core/document/Paragraph.h"

int Paragraph::wordCount() const
{
    return ParagraphUtils::estimateWordCount(text);
}

int ParagraphUtils::estimateWordCount(const QString& text)
{
    int count = 0;
    bool inChinese = false;
    bool inEnglish = false;

    for (const QChar& ch : text) {
        if (ch.unicode() >= 0x4E00 && ch.unicode() <= 0x9FFF) {
            if (!inChinese) { ++count; inChinese = true; }
            inEnglish = false;
        } else if (ch.isLetterOrNumber()) {
            if (!inEnglish) { ++count; inEnglish = true; }
            inChinese = false;
        } else {
            inChinese = false;
            inEnglish = false;
        }
    }
    return count;
}
