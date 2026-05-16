#ifndef WRITESMART_SIDEBAR_PANEL_H
#define WRITESMART_SIDEBAR_PANEL_H

#include <QWidget>
#include <QTreeView>
#include <QTabWidget>
#include <QStandardItemModel>
#include <QPushButton>
#include <memory>

class Document;

class SidebarPanel : public QWidget {
    Q_OBJECT

public:
    explicit SidebarPanel(QWidget* parent = nullptr);
    ~SidebarPanel() override;

    void setDocument(std::shared_ptr<Document> doc);
    void refreshAll();

signals:
    void chapterSelected(const QString& chapterId);
    void characterSelected(const QString& characterId);

private:
    void setupUI();
    void setupConnections();
    void refreshOutline();
    void refreshCharacters();
    void onAddChapter();
    void onRemoveChapter();
    void onAddCharacter();
    void onRemoveCharacter();

    QTabWidget* m_tabWidget = nullptr;

    QTreeView* m_outlineTree = nullptr;
    QStandardItemModel* m_outlineModel = nullptr;
    QPushButton* m_addChapterBtn = nullptr;
    QPushButton* m_removeChapterBtn = nullptr;

    QTreeView* m_characterTree = nullptr;
    QStandardItemModel* m_characterModel = nullptr;
    QPushButton* m_addCharBtn = nullptr;
    QPushButton* m_removeCharBtn = nullptr;

    QWidget* m_settingsTab = nullptr;

    std::shared_ptr<Document> m_document;
};

#endif
