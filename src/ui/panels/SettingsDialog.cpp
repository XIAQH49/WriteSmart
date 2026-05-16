#include "ui/panels/SettingsDialog.h"
#include "core/ai/providers/OpenAIProvider.h"
#include "core/ai/providers/ClaudeProvider.h"
#include "core/ai/providers/CustomProvider.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QApplication>
#include <QStyle>
#include <QJsonDocument>

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("设置");
    setMinimumWidth(500);
    setModal(true);
    setupUI();
    onProviderChanged(0);
}

void SettingsDialog::setupUI()
{
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setSpacing(12);

    // ========================
    // AI API 配置组
    // ========================
    auto* aiGroup = new QGroupBox("AI API 配置", this);
    auto* form = new QFormLayout(aiGroup);
    form->setSpacing(8);

    m_providerCombo = new QComboBox(this);
    m_providerCombo->addItem("OpenAI (GPT-4o)", "openai");
    m_providerCombo->addItem("Anthropic Claude", "claude");
    m_providerCombo->addItem("自定义 API", "custom");
    form->addRow("AI 提供商:", m_providerCombo);

    m_apiKeyEdit = new QLineEdit(this);
    m_apiKeyEdit->setPlaceholderText("sk-... 或 claude-...");
    m_apiKeyEdit->setEchoMode(QLineEdit::Password);
    form->addRow("API Key:", m_apiKeyEdit);

    m_baseUrlEdit = new QLineEdit(this);
    m_baseUrlEdit->setPlaceholderText("https://api.openai.com");
    form->addRow("Base URL:", m_baseUrlEdit);

    m_modelEdit = new QLineEdit(this);
    m_modelEdit->setPlaceholderText("gpt-4o");
    form->addRow("模型:", m_modelEdit);

    m_temperatureSpinner = new QDoubleSpinBox(this);
    m_temperatureSpinner->setRange(0.0, 2.0);
    m_temperatureSpinner->setSingleStep(0.1);
    m_temperatureSpinner->setValue(0.7);
    m_temperatureSpinner->setDecimals(1);
    form->addRow("Temperature:", m_temperatureSpinner);

    m_maxTokensSpinner = new QSpinBox(this);
    m_maxTokensSpinner->setRange(256, 131072);
    m_maxTokensSpinner->setSingleStep(512);
    m_maxTokensSpinner->setValue(4096);
    form->addRow("最大 Tokens:", m_maxTokensSpinner);

    rootLayout->addWidget(aiGroup);

    // ========================
    // 说明文字
    // ========================
    auto* infoLabel = new QLabel(this);
    infoLabel->setWordWrap(true);
    infoLabel->setText("<span style='color:#6c7086;'>"
        "OpenAI: <a href='https://platform.openai.com/api-keys'>获取 API Key</a><br>"
        "Claude: <a href='https://console.anthropic.com/'>获取 API Key</a><br>"
        "自定义 API: 支持 Ollama、LM Studio、Groq 等 OpenAI 兼容接口<br>"
        "API Key 仅保存在本地 config.json 中，不会上传。"
        "</span>");
    infoLabel->setOpenExternalLinks(true);
    rootLayout->addWidget(infoLabel);

    rootLayout->addStretch();

    // ========================
    // 底部按钮
    // ========================
    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    m_testBtn = new QPushButton("测试连接", this);
    m_testBtn->setFixedHeight(32);
    btnLayout->addWidget(m_testBtn);

    m_saveBtn = new QPushButton("保存设置", this);
    m_saveBtn->setFixedHeight(32);
    m_saveBtn->setDefault(true);
    btnLayout->addWidget(m_saveBtn);

    rootLayout->addLayout(btnLayout);

    connect(m_providerCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::onProviderChanged);
    connect(m_testBtn, &QPushButton::clicked, this, &SettingsDialog::onTestConnection);
    connect(m_saveBtn, &QPushButton::clicked, this, &SettingsDialog::onSave);
}

void SettingsDialog::onProviderChanged(int index)
{
    QString provider = m_providerCombo->itemData(index).toString();

    if (provider == "openai") {
        m_baseUrlEdit->setText("https://api.openai.com");
        m_baseUrlEdit->setReadOnly(true);
        m_modelEdit->setPlaceholderText("gpt-4o");
    } else if (provider == "claude") {
        m_baseUrlEdit->setText("https://api.anthropic.com");
        m_baseUrlEdit->setReadOnly(true);
        m_modelEdit->setPlaceholderText("claude-3-5-sonnet-20241022");
    } else {
        m_baseUrlEdit->setText("http://localhost:11434");
        m_baseUrlEdit->setReadOnly(false);
        m_modelEdit->setPlaceholderText("llama3");
    }
}

void SettingsDialog::onTestConnection()
{
    m_testBtn->setEnabled(false);
    m_testBtn->setText("测试中...");

    QJsonObject config;
    config["apiKey"] = m_apiKeyEdit->text().trimmed();
    config["baseUrl"] = m_baseUrlEdit->text().trimmed();
    config["model"] = m_modelEdit->text().trimmed();
    config["temperature"] = m_temperatureSpinner->value();
    config["maxTokens"] = m_maxTokensSpinner->value();

    // 简单检查
    if (config["apiKey"].toString().isEmpty() && config["baseUrl"].toString().contains("api.openai")) {
        m_testBtn->setEnabled(true);
        m_testBtn->setText("❌ 需要 API Key");
        return;
    }

    // 使用异步方式快速检测
    AIProviderPtr provider;
    QString type = m_providerCombo->currentData().toString();
    if (type == "openai") provider = std::make_shared<OpenAIProvider>();
    else if (type == "claude") provider = std::make_shared<ClaudeProvider>();
    else provider = std::make_shared<CustomProvider>();

    if (provider->configure(config)) {
        // 发起最小请求测试
        ChatRequest req;
        req.model = config["model"].toString();
        req.temperature = 0.1;
        req.maxTokens = 10;
        ChatMessage msg;
        msg.role = "user";
        msg.content = "reply ok";
        req.messages.append(msg);

        provider->chat(req, [this](bool success, const ChatResponse&, const QString& error) {
            if (success) {
                QMessageBox::information(this, "测试连接", "✅ API 连接成功！");
            } else {
                QMessageBox::warning(this, "测试连接", "❌ 连接失败:\n" + error);
            }
            m_testBtn->setEnabled(true);
            m_testBtn->setText("测试连接");
        });
    } else {
        m_testBtn->setEnabled(true);
        m_testBtn->setText("测试连接");
        QMessageBox::warning(this, "测试连接", "配置不完整");
    }
}

void SettingsDialog::onSave()
{
    QJsonObject config;
    config["provider"] = m_providerCombo->currentData().toString();
    config["apiKey"] = m_apiKeyEdit->text().trimmed();
    config["baseUrl"] = m_baseUrlEdit->text().trimmed();
    config["model"] = m_modelEdit->text().trimmed();
    config["temperature"] = m_temperatureSpinner->value();
    config["maxTokens"] = m_maxTokensSpinner->value();

    emit configChanged(config);
    accept();
}

QJsonObject SettingsDialog::aiConfig() const
{
    QJsonObject config;
    config["provider"] = m_providerCombo->currentData().toString();
    config["apiKey"] = m_apiKeyEdit->text().trimmed();
    config["baseUrl"] = m_baseUrlEdit->text().trimmed();
    config["model"] = m_modelEdit->text().trimmed();
    config["temperature"] = m_temperatureSpinner->value();
    config["maxTokens"] = m_maxTokensSpinner->value();
    return config;
}

void SettingsDialog::setAiConfig(const QJsonObject& config)
{
    QString provider = config["provider"].toString();
    int idx = m_providerCombo->findData(provider);
    if (idx >= 0) m_providerCombo->setCurrentIndex(idx);

    m_apiKeyEdit->setText(config["apiKey"].toString());
    m_baseUrlEdit->setText(config["baseUrl"].toString());
    m_modelEdit->setText(config["model"].toString());
    if (config.contains("temperature")) m_temperatureSpinner->setValue(config["temperature"].toDouble());
    if (config.contains("maxTokens")) m_maxTokensSpinner->setValue(config["maxTokens"].toInt());
}
