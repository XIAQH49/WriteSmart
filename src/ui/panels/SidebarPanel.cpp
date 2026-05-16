#include "ui/panels/SidebarPanel.h"
#include "core/document/Document.h"
#include "utils/StringUtils.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QInputDialog>
#include <QMessageBox>

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

    // ========================
    // 大纲 Tab
    // ========================
    QWidget* outlineTab = new QWidget(this);
    auto* outlineLayout = new QVBoxLayout(outlineTab);
    outlineLayout->setContentsMargins(4, 4, 4, 4);

    m_outlineModel = new QStandardItemModel(this);
    m_outlineTree = new QTreeView(this);
    m_outlineTree->setModel(m_outlineModel);
    m_outlineTree->setHeaderHidden(true);
    m_outlineTree->setIndentation(16);
    m_outlineTree->setAnimated(true);
    m_outlineTree->setSelectionMode(QAbstractItemView::SingleSelection);
    outlineLayout->addWidget(m_outlineTree, 1);

    auto* outlineBtnLayout = new QHBoxLayout();
    m_addChapterBtn = new QPushButton("+ 章", this);
    m_addChapterBtn->setFixedHeight(24);
    m_addChapterBtn->setToolTip("添加新章节");
    m_removeChapterBtn = new QPushButton("- 章", this);
    m_removeChapterBtn->setFixedHeight(24);
    m_removeChapterBtn->setToolTip("删除选中章节");
    outlineBtnLayout->addWidget(m_addChapterBtn);
    outlineBtnLayout->addWidget(m_removeChapterBtn);
    outlineLayout->addLayout(outlineBtnLayout);

    m_tabWidget->addTab(outlineTab, "大纲");

    // ========================
    // 人物 Tab
    // ========================
    QWidget* charTab = new QWidget(this);
    auto* charLayout = new QVBoxLayout(charTab);
    charLayout->setContentsMargins(4, 4, 4, 4);

    m_characterModel = new QStandardItemModel(this);
    m_characterTree = new QTreeView(this);
    m_characterTree->setModel(m_characterModel);
    m_characterTree->setHeaderHidden(true);
    m_characterTree->setSelectionMode(QAbstractItemView::SingleSelection);
    charLayout->addWidget(m_characterTree, 1);

    auto* charBtnLayout = new QHBoxLayout();
    m_addCharBtn = new QPushButton("+ 角色", this);
    m_addCharBtn->setFixedHeight(24);
    m_removeCharBtn = new QPushButton("- 角色", this);
    m_removeCharBtn->setFixedHeight(24);
    charBtnLayout->addWidget(m_addCharBtn);
    charBtnLayout->addWidget(m_removeCharBtn);
    charLayout->addLayout(charBtnLayout);

    m_tabWidget->addTab(charTab, "人物");

    // ========================
    // 设定 Tab（独立 Model，不污染人物）
    // ========================
    QWidget* settingsTab = new QWidget(this);
    auto* settingsLayout = new QVBoxLayout(settingsTab);
    settingsLayout->setContentsMargins(4, 4, 4, 4);

    m_settingsModel = new QStandardItemModel(this);
    m_settingsTree = new QTreeView(this);
    m_settingsTree->setModel(m_settingsModel);
    m_settingsTree->setHeaderHidden(true);
    m_settingsTree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_settingsTree->setRootIsDecorated(false);
    settingsLayout->addWidget(m_settingsTree, 1);

    auto* settingsBtnLayout = new QHBoxLayout();
    m_addSettingBtn = new QPushButton("+ 条目", this);
    m_addSettingBtn->setFixedHeight(24);
    m_addSettingBtn->setToolTip("新建设定条目（世界观、道具等）");
    m_removeSettingBtn = new QPushButton("- 条目", this);
    m_removeSettingBtn->setFixedHeight(24);
    m_removeSettingBtn->setToolTip("删除选中的设定条目");
    settingsBtnLayout->addWidget(m_addSettingBtn);
    settingsBtnLayout->addWidget(m_removeSettingBtn);
    settingsLayout->addLayout(settingsBtnLayout);

    m_tabWidget->addTab(settingsTab, "设定");

    layout->addWidget(m_tabWidget);
}

void SidebarPanel::setupConnections()
{
    connect(m_outlineTree, &QTreeView::clicked, this, [this](const QModelIndex& index) {
        if (!index.isValid()) return;
        QString chapterId = m_outlineModel->itemFromIndex(index)->data(Qt::UserRole).toString();
        if (!chapterId.isEmpty()) emit chapterSelected(chapterId);
    });

    connect(m_characterTree, &QTreeView::clicked, this, [this](const QModelIndex& index) {
        if (!index.isValid()) return;
        QString characterId = m_characterModel->itemFromIndex(index)->data(Qt::UserRole).toString();
        if (!characterId.isEmpty()) emit characterSelected(characterId);
    });

    connect(m_addChapterBtn, &QPushButton::clicked, this, &SidebarPanel::onAddChapter);
    connect(m_removeChapterBtn, &QPushButton::clicked, this, &SidebarPanel::onRemoveChapter);
    connect(m_addCharBtn, &QPushButton::clicked, this, &SidebarPanel::onAddCharacter);
    connect(m_removeCharBtn, &QPushButton::clicked, this, &SidebarPanel::onRemoveCharacter);
    connect(m_addSettingBtn, &QPushButton::clicked, this, &SidebarPanel::onAddSetting);
    connect(m_removeSettingBtn, &QPushButton::clicked, this, &SidebarPanel::onRemoveSetting);
}

void SidebarPanel::setDocument(std::shared_ptr<Document> doc)
{
    m_document = std::move(doc);
    refreshAll();
}

void SidebarPanel::refreshAll()
{
    refreshOutline();
    refreshCharacters();
}

void SidebarPanel::refreshOutline()
{
    m_outlineModel->clear();
    if (!m_document) return;

    for (const auto& ch : m_document->chapters()) {
        auto* item = new QStandardItem(ch.title.isEmpty() ? "(无标题)" : ch.title);
        item->setData(ch.id, Qt::UserRole);
        item->setEditable(true);
        item->setToolTip(QString("%1 段落 · %2 字").arg(ch.paragraphs.size()).arg(ch.wordCount));
        m_outlineModel->appendRow(item);
    }

    if (m_document->outlineRoot()) {
        std::function<void(std::shared_ptr<OutlineNode>, QStandardItem*)> addNode =
            [&](std::shared_ptr<OutlineNode> node, QStandardItem* parent) {
                if (node->title.isEmpty() && node->children.isEmpty()) return;
                auto* item = new QStandardItem(node->title.isEmpty() ? "..." : node->title);
                item->setData(node->id, Qt::UserRole);
                item->setData(node->linkedChapterId, Qt::UserRole + 1);
                item->setEditable(false);
                if (parent) parent->appendRow(item);
                else m_outlineModel->appendRow(item);
                for (auto& child : node->children) addNode(child, item);
            };

        for (auto& child : m_document->outlineRoot()->children) {
            addNode(child, nullptr);
        }
    }

    m_outlineTree->expandAll();
}

void SidebarPanel::refreshCharacters()
{
    m_characterModel->clear();
    if (!m_document) return;

    for (const auto& ch : m_document->characters()) {
        auto* item = new QStandardItem(ch.name);
        item->setData(ch.id, Qt::UserRole);
        item->setEditable(true);
        if (!ch.description.isEmpty()) item->setToolTip(ch.description);
        m_characterModel->appendRow(item);
    }
}

void SidebarPanel::onAddChapter()
{
    if (!m_document) return;
    bool ok;
    QString title = QInputDialog::getText(this, "新建章节", "章节标题:", QLineEdit::Normal, "", &ok);
    if (!ok || title.isEmpty()) return;

    Chapter ch;
    ch.id = StringUtils::generateId("ch");
    ch.title = title;
    m_document->addChapter(ch);
    refreshOutline();
}

void SidebarPanel::onRemoveChapter()
{
    if (!m_document) return;
    QModelIndex idx = m_outlineTree->currentIndex();
    if (!idx.isValid()) return;

    QString chapterId = m_outlineModel->itemFromIndex(idx)->data(Qt::UserRole).toString();
    if (chapterId.isEmpty()) return;

    auto r = QMessageBox::question(this, "删除章节",
        "确定要删除该章节吗？此操作不可撤销。",
        QMessageBox::Yes | QMessageBox::No);
    if (r == QMessageBox::Yes) {
        m_document->removeChapter(chapterId);
        refreshOutline();
    }
}

void SidebarPanel::onAddCharacter()
{
    if (!m_document) return;
    bool ok;
    QString name = QInputDialog::getText(this, "新建角色", "角色姓名:", QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) return;

    Character ch;
    ch.id = StringUtils::generateId("char");
    ch.name = name;
    ch.description = QInputDialog::getText(this, "角色描述", name + " 的描述:", QLineEdit::Normal, "", &ok);
    m_document->addCharacter(ch);
    refreshCharacters();
}

void SidebarPanel::onRemoveCharacter()
{
    if (!m_document) return;
    QModelIndex idx = m_characterTree->currentIndex();
    if (!idx.isValid()) return;

    QString charId = m_characterModel->itemFromIndex(idx)->data(Qt::UserRole).toString();
    if (charId.isEmpty()) return;

    auto& chars = m_document->characters();
    chars.erase(std::remove_if(chars.begin(), chars.end(),
        [&](const Character& c) { return c.id == charId; }), chars.end());
    refreshCharacters();
}

void SidebarPanel::onAddSetting()
{
    bool ok;
    QString key = QInputDialog::getText(this, "新建设定", "条目名称:", QLineEdit::Normal, "", &ok);
    if (!ok || key.isEmpty()) return;

    auto* item = new QStandardItem(key);
    item->setEditable(true);
    item->setData(StringUtils::generateId("setting"), Qt::UserRole);
    m_settingsModel->appendRow(item);
}

void SidebarPanel::onRemoveSetting()
{
    QModelIndex idx = m_settingsTree->currentIndex();
    if (!idx.isValid()) return;

    auto r = QMessageBox::question(this, "删除设定条目",
        "确定要删除 \"" + m_settingsModel->itemFromIndex(idx)->text() + "\" 吗？",
        QMessageBox::Yes | QMessageBox::No);
    if (r == QMessageBox::Yes) {
        m_settingsModel->removeRow(idx.row());
    }
}
