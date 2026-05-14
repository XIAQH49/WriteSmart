#ifndef WRITESMART_SIDEBAR_PANEL_H
#define WRITESMART_SIDEBAR_PANEL_H

#include <QWidget>
#include <QTreeView>
#include <QTabWidget>
#include <QStandardItemModel>
#include <memory>

class Document;

enum class SidebarTab {
    Outline = 0,
    Characters,
    Settings_Notes,
    Count
};

class SidebarPanel : public QWidget {
    Q_OBJECT

public:
    explicit SidebarPanel(QWidget* parent = nullptr);
    ~SidebarPanel() override;

    void setDocument(std::shared_ptr<Document> doc);

signals:
    void chapterSelected(const QString& chapterId);
    void characterSelected(const QString& characterId);

private:
    void setupUI();
    void setupConnections();
    void refreshOutline();
    void refreshCharacters();

    QTabWidget* m_tabWidget = nullptr;

    QTreeView* m_outlineTree = nullptr;
    QStandardItemModel* m_outlineModel = nullptr;

    QTreeView* m_characterTree = nullptr;
    QStandardItemModel* m_characterModel = nullptr;

    QWidget* m_settingsTab = nullptr;

    std::shared_ptr<Document> m_document;
};

#endif // WRITESMART_SIDEBAR_PANEL_H
