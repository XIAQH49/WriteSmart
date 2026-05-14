#ifndef WRITESMART_MAIN_WINDOW_H
#define WRITESMART_MAIN_WINDOW_H

#include <QMainWindow>
#include <QSplitter>

class SidebarPanel;
class EditorPanel;
class ChatPanel;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onDocumentChanged();
    void onAIRequested(const QString& prompt, const QString& context);

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void setupConnections();
    void saveWindowState();
    void restoreWindowState();

    QSplitter* m_mainSplitter = nullptr;
    SidebarPanel* m_sidebar = nullptr;
    EditorPanel* m_editor = nullptr;
    ChatPanel* m_chatPanel = nullptr;
};

#endif // WRITESMART_MAIN_WINDOW_H
