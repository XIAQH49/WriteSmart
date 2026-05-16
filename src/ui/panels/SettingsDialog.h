#ifndef WRITESMART_SETTINGS_DIALOG_H
#define WRITESMART_SETTINGS_DIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QJsonObject>
#include <memory>

#include "core/ai/AIProvider.h"

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog() override = default;

    QJsonObject aiConfig() const;
    void setAiConfig(const QJsonObject& config);

signals:
    void configChanged(const QJsonObject& aiConfig);

private:
    void setupUI();
    void onProviderChanged(int index);
    void onTestConnection();
    void onSave();

    QComboBox* m_providerCombo = nullptr;
    QLineEdit* m_apiKeyEdit = nullptr;
    QLineEdit* m_baseUrlEdit = nullptr;
    QLineEdit* m_modelEdit = nullptr;
    QDoubleSpinBox* m_temperatureSpinner = nullptr;
    QSpinBox* m_maxTokensSpinner = nullptr;
    QPushButton* m_testBtn = nullptr;
    QPushButton* m_saveBtn = nullptr;
};

#endif
