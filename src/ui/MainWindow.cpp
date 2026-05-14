#include "ui/MainWindow.h"
#include "ui/panels/SidebarPanel.h"
#include "ui/panels/EditorPanel.h"
#include "ui/panels/ChatPanel.h"
#include "app/Config.h"
#include "ui/themes/ThemeManager.h"
#include <QMenuBar>
#include <QStatusBar>
#include <QCloseEvent>
#include <QFile>
#include <QApplication>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setupUI();
    setupMenuBar();
    setupStatusBar();
    setupConnections();
    restoreWindowState();

    setWindowTitle("WriteSmart 2.0 — 智能文学创作");

    // 加载主题
    QString theme = Config::instance().theme();
    ThemeManager::instance().loadTheme(theme);
    if (!ThemeManager::instance().styleSheet().isEmpty()) {
        setStyleSheet(ThemeManager::instance().styleSheet());
    } else {
        QFile qss(":/styles/default.qss");
        if (qss.open(QIODevice::ReadOnly)) {
            setStyleSheet(QString::fromUtf8(qss.readAll()));
            qss.close();
        }
    }
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

    setCentralWidget(m_mainSplitter);
}

void MainWindow::setupMenuBar()
{
    QMenu* fileMenu = menuBar()->addMenu("文件(&F)");

    QAction* newAction = fileMenu->addAction("新建(&N)");
    newAction->setShortcut(QKeySequence("Ctrl+N"));

    QAction* openAction = fileMenu->addAction("打开(&O)");
    openAction->setShortcut(QKeySequence("Ctrl+O"));

    QAction* saveAction = fileMenu->addAction("保存(&S)");
    saveAction->setShortcut(QKeySequence("Ctrl+S"));

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
    helpMenu->addAction("关于 WriteSmart(&A)");
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage("就绪");
}

void MainWindow::setupConnections()
{
    connect(m_editor, &EditorPanel::textSelected, m_chatPanel, &ChatPanel::injectContext);
    connect(m_sidebar, &SidebarPanel::chapterSelected, m_editor, &EditorPanel::navigateToChapter);
}

void MainWindow::onDocumentChanged()
{
    statusBar()->showMessage("文档已修改", 3000);
}

void MainWindow::onAIRequested(const QString& /*prompt*/, const QString& context)
{
    if (!context.isEmpty()) {
        m_chatPanel->injectContext(context);
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    saveWindowState();
    event->accept();
}

void MainWindow::saveWindowState()
{
    auto& ws = Config::instance().windowState();
    ws.x = x();
    ws.y = y();
    ws.width = width();
    ws.height = height();
    ws.maximized = isMaximized();
    ws.splitterSizes = m_mainSplitter->sizes();
}

void MainWindow::restoreWindowState()
{
    const auto& ws = Config::instance().windowState();
    resize(ws.width, ws.height);
    move(ws.x, ws.y);
    if (ws.maximized) {
        showMaximized();
    }
    if (!ws.splitterSizes.isEmpty()) {
        m_mainSplitter->setSizes(ws.splitterSizes);
    }
}
