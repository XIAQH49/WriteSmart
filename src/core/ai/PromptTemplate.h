#ifndef WRITESMART_PROMPT_TEMPLATE_H
#define WRITESMART_PROMPT_TEMPLATE_H

#include <QString>
#include <QMap>
#include <QJsonObject>

class PromptTemplate {
public:
    explicit PromptTemplate(const QString& name = QString());

    QString name() const;
    void setName(const QString& name);

    QString systemPrompt() const;
    void setSystemPrompt(const QString& prompt);

    QString format(const QMap<QString, QString>& variables) const;

    QJsonObject toJson() const;
    static PromptTemplate fromJson(const QJsonObject& json);

    static PromptTemplate defaultWriter();
    static PromptTemplate defaultEditor();
    static PromptTemplate defaultOutliner();

private:
    QString m_name;
    QString m_systemPrompt;
};

#endif // WRITESMART_PROMPT_TEMPLATE_H
