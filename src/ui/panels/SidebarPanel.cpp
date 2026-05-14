#include "ui/panels/SidebarPanel.h"
#include "core/document/Document.h"
#include <QVBoxLayout>
#include <QPushButton>

SidebarPanel::SidebarPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    setupConnections();
    setMinimumWidth(180);
}

SidebarPanel::~SidebarPanel() = default;

void SidebarPanel::setupUI()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabPosition(QTabWidget::North);

    // 大纲 Tab
    m_outlineModel = new QStandardItemModel(this);
    m_outlineModel->setHorizontalHeaderLabels({"大纲"});
    m_outlineTree = new QTreeView(this);
    m_outlineTree->setModel(m_outlineModel);
    m_outlineTree->setHeaderHidden(true);
    m_outlineTree->setIndentation(16);
    m_outlineTree->setAnimated(true);
    m_tabWidget->addTab(m_outlineTree, "大纲");

    // 人物 Tab
    m_characterModel = new QStandardItemModel(this);
    m_characterModel->setHorizontalHeaderLabels({"人物"});
    m_characterTree = new QTreeView(this);
    m_characterTree->setModel(m_characterModel);
    m_characterTree->setHeaderHidden(true);
    m_tabWidget->addTab(m_characterTree, "人物");

    // 设定 Tab
    m_settingsTab = new QWidget(this);
    auto* settingsLayout = new QVBoxLayout(m_settingsTab);
    auto* addSettingBtn = new QPushButton("新建设定", m_settingsTab);
    settingsLayout->addWidget(addSettingBtn);
    settingsLayout->addStretch();
    m_tabWidget->addTab(m_settingsTab, "设定");

    layout->addWidget(m_tabWidget);
}

void SidebarPanel::setupConnections()
{
    connect(m_outlineTree, &QTreeView::clicked, this, [this](const QModelIndex& index) {
        if (!index.isValid()) return;
        QString chapterId = m_outlineModel->itemFromIndex(index)->data().toString();
        if (!chapterId.isEmpty()) {
            emit chapterSelected(chapterId);
        }
    });

    connect(m_characterTree, &QTreeView::clicked, this, [this](const QModelIndex& index) {
        if (!index.isValid()) return;
        QString characterId = m_characterModel->itemFromIndex(index)->data().toString();
        if (!characterId.isEmpty()) {
            emit characterSelected(characterId);
        }
    });
}

void SidebarPanel::setDocument(std::shared_ptr<Document> doc)
{
    m_document = std::move(doc);
    refreshOutline();
    refreshCharacters();
}

void SidebarPanel::refreshOutline()
{
    m_outlineModel->clear();
    if (!m_document || !m_document->outlineRoot()) return;

    std::function<void(std::shared_ptr<OutlineNode>, QStandardItem*)> addNode =
        [&](std::shared_ptr<OutlineNode> node, QStandardItem* parent) {
            auto* item = new QStandardItem(node->title);
            item->setData(node->id);
            item->setEditable(false);
            if (parent) {
                parent->appendRow(item);
            } else {
                m_outlineModel->appendRow(item);
            }
            for (auto& child : node->children) {
                addNode(child, item);
            }
        };

    addNode(m_document->outlineRoot(), nullptr);
    m_outlineTree->expandAll();
}

void SidebarPanel::refreshCharacters()
{
    m_characterModel->clear();
    if (!m_document) return;

    for (const auto& ch : m_document->characters()) {
        auto* item = new QStandardItem(ch.name);
        item->setData(ch.id);
        item->setEditable(false);
        if (!ch.description.isEmpty()) {
            item->setToolTip(ch.description);
        }
        m_characterModel->appendRow(item);
    }
}
