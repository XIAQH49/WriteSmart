#ifndef WRITESMART_STRING_UTILS_H
#define WRITESMART_STRING_UTILS_H

#include <QString>
#include <QStringList>

class StringUtils {
public:
    static int countChineseWords(const QString& text);
    static int countEnglishWords(const QString& text);
    static int totalWordCount(const QString& text);

    static QString truncate(const QString& text, int maxLen, const QString& suffix = "...");
    static QString generateId(const QString& prefix = QString());
    static QString elapsedString(qint64 ms);
};

#endif // WRITESMART_STRING_UTILS_H
