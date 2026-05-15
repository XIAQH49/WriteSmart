#ifndef WRITESMART_MAIN_WINDOW_H
#define WRITESMART_MAIN_WINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <memory>

class SidebarPanel;
class EditorPanel;
class ChatPanel;
class Document;
class DocumentManager;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void openDocument(const QString& filePath);
    void saveDocumentIfNeeded();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onNewDocument();
    void onOpenDocument();
    void onSaveDocument();
    void onDocumentChanged();
    void onWordCountChanged(int count);
    void onAIRequested(const QString& prompt, const QString& context);

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void setupConnections();
    void setupShortcuts();
    void loadTheme();
    void saveWindowState();
    void restoreWindowState();
    void updateDocumentTitle();

    QSplitter* m_mainSplitter = nullptr;
    SidebarPanel* m_sidebar = nullptr;
    EditorPanel* m_editor = nullptr;
    ChatPanel* m_chatPanel = nullptr;

    std::unique_ptr<DocumentManager> m_docManager;
    QAction* m_saveAction = nullptr;
};

#endif
