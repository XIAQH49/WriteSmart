#include "core/ai/PromptTemplate.h"

PromptTemplate::PromptTemplate(const QString& name)
    : m_name(name)
{
}

QString PromptTemplate::name() const { return m_name; }
void PromptTemplate::setName(const QString& name) { m_name = name; }

QString PromptTemplate::systemPrompt() const { return m_systemPrompt; }
void PromptTemplate::setSystemPrompt(const QString& prompt) { m_systemPrompt = prompt; }

QString PromptTemplate::format(const QMap<QString, QString>& variables) const
{
    QString result = m_systemPrompt;
    for (auto it = variables.begin(); it != variables.end(); ++it) {
        result.replace("{" + it.key() + "}", it.value());
    }
    return result;
}

QJsonObject PromptTemplate::toJson() const
{
    QJsonObject obj;
    obj["name"] = m_name;
    obj["systemPrompt"] = m_systemPrompt;
    return obj;
}

PromptTemplate PromptTemplate::fromJson(const QJsonObject& json)
{
    PromptTemplate tmpl;
    tmpl.m_name = json["name"].toString();
    tmpl.m_systemPrompt = json["systemPrompt"].toString();
    return tmpl;
}

PromptTemplate PromptTemplate::defaultWriter()
{
    PromptTemplate tmpl("default_writer");
    tmpl.setSystemPrompt("你是一位专业的文学创作助手，帮助用户进行小说、散文等文学创作。");
    return tmpl;
}

PromptTemplate PromptTemplate::defaultEditor()
{
    PromptTemplate tmpl("default_editor");
    tmpl.setSystemPrompt("你是一位文字编辑，帮助用户润色文本、检查语法错误、优化表达。");
    return tmpl;
}

PromptTemplate PromptTemplate::defaultOutliner()
{
    PromptTemplate tmpl("default_outliner");
    tmpl.setSystemPrompt("你是一位大纲规划师，帮助用户梳理文章结构、生成章节大纲。");
    return tmpl;
}
