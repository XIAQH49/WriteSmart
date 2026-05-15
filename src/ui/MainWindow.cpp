#include "ui/MainWindow.h"
#include "ui/panels/SidebarPanel.h"
#include "ui/panels/EditorPanel.h"
#include "ui/panels/ChatPanel.h"
#include "core/document/Document.h"
#include "core/document/DocumentManager.h"
#include "utils/Config.h"
#include "ui/themes/ThemeManager.h"
#include <QMenuBar>
#include <QStatusBar>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QApplication>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    m_docManager = std::make_unique<DocumentManager>(this);
    setupUI();
    setupMenuBar();
    setupStatusBar();
    setupConnections();
    loadTheme();
    restoreWindowState();

    setWindowTitle("WriteSmart 2.0");
    resize(1400, 900);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI()
{
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    m_mainSplitter->setHandleWidth(3);
    m_mainSplitter->setChildrenCollapsible(false);

    m_sidebar  = new SidebarPanel(m_mainSplitter);
    m_editor   = new EditorPanel(m_mainSplitter);
    m_chatPanel = new ChatPanel(m_mainSplitter);

    m_mainSplitter->addWidget(m_sidebar);
    m_mainSplitter->addWidget(m_editor);
    m_mainSplitter->addWidget(m_chatPanel);

    m_mainSplitter->setStretchFactor(0, 0);
    m_mainSplitter->setStretchFactor(1, 1);
    m_mainSplitter->setStretchFactor(2, 0);

    m_mainSplitter->setSizes({220, 830, 350});

    setCentralWidget(m_mainSplitter);
}

void MainWindow::setupMenuBar()
{
    QMenu* fileMenu = menuBar()->addMenu("文件(&F)");

    QAction* newAction = fileMenu->addAction("新建(&N)");
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::onNewDocument);

    QAction* openAction = fileMenu->addAction("打开(&O)");
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::onOpenDocument);

    m_saveAction = fileMenu->addAction("保存(&S)");
    m_saveAction->setShortcut(QKeySequence::Save);
    m_saveAction->setEnabled(false);
    connect(m_saveAction, &QAction::triggered, this, &MainWindow::onSaveDocument);

    fileMenu->addSeparator();

    QAction* exitAction = fileMenu->addAction("退出(&X)");
    exitAction->setShortcut(QKeySequence("Ctrl+Q"));
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    QMenu* viewMenu = menuBar()->addMenu("视图(&V)");
    QAction* toggleSidebar = viewMenu->addAction("切换侧边栏");
    toggleSidebar->setCheckable(true);
    toggleSidebar->setChecked(true);
    connect(toggleSidebar, &QAction::toggled, m_sidebar, &QWidget::setVisible);

    QAction* toggleChat = viewMenu->addAction("切换对话栏");
    toggleChat->setCheckable(true);
    toggleChat->setChecked(true);
    connect(toggleChat, &QAction::toggled, m_chatPanel, &QWidget::setVisible);

    QMenu* helpMenu = menuBar()->addMenu("帮助(&H)");
    QAction* aboutAction = helpMenu->addAction("关于 WriteSmart(&A)");
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, "WriteSmart 2.0",
            "智能文学创作客户端\n\n"
            "三段式写作工作区\n"
            "自定义 AI API\n"
            "C++ / Qt 6.9");
    });
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage("就绪  |  Ctrl+N 新建  |  Ctrl+O 打开  |  Ctrl+S 保存");
}

void MainWindow::setupConnections()
{
    connect(m_editor, &EditorPanel::textSelected, m_chatPanel, &ChatPanel::injectContext);
    connect(m_editor, &EditorPanel::wordCountChanged, this, &MainWindow::onWordCountChanged);
    connect(m_sidebar, &SidebarPanel::chapterSelected, m_editor, &EditorPanel::navigateToChapter);
    connect(m_docManager.get(), &DocumentManager::documentLoaded, this, [this](auto doc) {
        m_sidebar->setDocument(doc);
        m_editor->setDocument(doc);
        m_saveAction->setEnabled(true);
        updateDocumentTitle();
    });
    connect(m_docManager.get(), &DocumentManager::modificationChanged, this, [this](bool) {
        updateDocumentTitle();
    });
}

void MainWindow::loadTheme()
{
    QString theme = Config::instance().theme();
    ThemeManager::instance().loadTheme(theme);
    QString qss = ThemeManager::instance().styleSheet();
    if (qss.isEmpty()) {
        QFile qssFile(":/styles/default.qss");
        if (qssFile.open(QIODevice::ReadOnly)) qss = qssFile.readAll();
    }
    if (!qss.isEmpty()) setStyleSheet(qss);
}

void MainWindow::onNewDocument()
{
    if (m_docManager->isModified()) {
        auto r = QMessageBox::question(this, "未保存", "当前文档有修改，是否保存？",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (r == QMessageBox::Save) onSaveDocument();
        if (r == QMessageBox::Cancel) return;
    }

    auto doc = std::make_shared<Document>();
    doc->setTitle("未命名文档");
    m_docManager->setCurrentDocument(doc);
    m_sidebar->setDocument(doc);
    m_editor->setDocument(doc);
    m_saveAction->setEnabled(true);
    statusBar()->showMessage("新建文档", 3000);
    updateDocumentTitle();
}

void MainWindow::onOpenDocument()
{
    QString path = QFileDialog::getOpenFileName(this, "打开文档", QString(),
        "WriteSmart 文档 (*.wrs);;Markdown (*.md);;所有文件 (*)");
    if (path.isEmpty()) return;
    openDocument(path);
}

void MainWindow::openDocument(const QString& filePath)
{
    auto doc = m_docManager->openDocument(filePath);
    if (doc) {
        Config::instance().setLastDocumentPath(filePath);
        Config::instance().addRecentDocument(filePath);
        statusBar()->showMessage("已打开: " + filePath, 3000);
    }
}

void MainWindow::onSaveDocument()
{
    if (!m_docManager->currentDocument()) return;

    QString path = m_docManager->currentDocument()->filePath();
    if (path.isEmpty()) {
        path = QFileDialog::getSaveFileName(this, "保存文档", "未命名.wrs",
            "WriteSmart 文档 (*.wrs);;Markdown (*.md)");
    }
    if (path.isEmpty()) return;

    if (m_docManager->saveDocument(path)) {
        Config::instance().setLastDocumentPath(path);
        Config::instance().addRecentDocument(path);
        statusBar()->showMessage("已保存: " + path, 3000);
    }
}

void MainWindow::saveDocumentIfNeeded()
{
    if (m_docManager->isModified() && m_docManager->currentDocument()) {
        onSaveDocument();
    }
}

void MainWindow::onDocumentChanged()
{
    statusBar()->showMessage("文档已修改", 2000);
}

void MainWindow::onWordCountChanged(int count)
{
    statusBar()->showMessage(QString("字数: %1").arg(count));
}

void MainWindow::onAIRequested(const QString& /*prompt*/, const QString& context)
{
    if (!context.isEmpty()) m_chatPanel->injectContext(context);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    saveWindowState();
    if (m_docManager->isModified()) {
        auto r = QMessageBox::question(this, "未保存", "文档已修改，保存后退出？",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (r == QMessageBox::Save) onSaveDocument();
        if (r == QMessageBox::Cancel) { event->ignore(); return; }
    }
    event->accept();
}

void MainWindow::saveWindowState()
{
    auto& ws = Config::instance().windowState();
    if (!isMaximized()) {
        ws.x = x(); ws.y = y(); ws.width = width(); ws.height = height();
    }
    ws.maximized = isMaximized();
    ws.splitterSizes = m_mainSplitter->sizes();
}

void MainWindow::restoreWindowState()
{
    const auto& ws = Config::instance().windowState();
    move(ws.x, ws.y);
    resize(ws.width, ws.height);
    if (ws.maximized) showMaximized();
    if (!ws.splitterSizes.isEmpty()) m_mainSplitter->setSizes(ws.splitterSizes);
}

void MainWindow::updateDocumentTitle()
{
    QString title = "WriteSmart 2.0";
    if (auto doc = m_docManager->currentDocument()) {
        title = doc->title() + " - WriteSmart 2.0";
        if (doc->isModified()) title = "* " + title;
    }
    setWindowTitle(title);
}
