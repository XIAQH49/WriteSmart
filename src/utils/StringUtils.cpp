#include "utils/StringUtils.h"
#include <QUuid>
#include <QStringBuilder>

int StringUtils::countChineseWords(const QString& text)
{
    int count = 0;
    for (const QChar& ch : text) {
        if (ch.unicode() >= 0x4E00 && ch.unicode() <= 0x9FFF) {
            ++count;
        }
    }
    return count;
}

int StringUtils::countEnglishWords(const QString& text)
{
    int count = 0;
    bool inWord = false;
    for (const QChar& ch : text) {
        if (ch.isLetterOrNumber()) {
            if (!inWord) { ++count; inWord = true; }
        } else {
            inWord = false;
        }
    }
    return count;
}

int StringUtils::totalWordCount(const QString& text)
{
    return countChineseWords(text) + countEnglishWords(text);
}

QString StringUtils::truncate(const QString& text, int maxLen, const QString& suffix)
{
    if (text.length() <= maxLen) return text;
    return text.left(maxLen) + suffix;
}

QString StringUtils::generateId(const QString& prefix)
{
    QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    return prefix.isEmpty() ? uuid : prefix % "_" % uuid;
}

QString StringUtils::elapsedString(qint64 ms)
{
    if (ms < 1000) return QString::number(ms) + "ms";
    if (ms < 60000) return QString::number(ms / 1000.0, 'f', 1) + "s";
    return QString::number(ms / 60000) + "m";
}
