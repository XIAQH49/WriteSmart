#ifndef WRITESMART_EDITOR_PANEL_H
#define WRITESMART_EDITOR_PANEL_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QTimer>
#include <memory>

class Document;
class Chapter;

class EditorPanel : public QWidget {
    Q_OBJECT

public:
    explicit EditorPanel(QWidget* parent = nullptr);
    ~EditorPanel() override;

    void setDocument(std::shared_ptr<Document> doc);
    void navigateToChapter(const QString& chapterId);

    QString selectedText() const;
    QString fullText() const;
    int currentWordCount() const;
    void syncEditsToDocument();

    void scrollToLine(int lineNumber);

signals:
    void wordCountChanged(int count);
    void textSelected(const QString& text);
    void documentModified();

private slots:
    void onTextChanged();
    void onAutoSave();
    void onSelectionChanged();

private:
    void setupUI();
    void setupConnections();
    void updateWordCount();
    void syncDocumentToEditor();
    int findChapterLine(const QString& chapterId) const;
    QString currentChapterId() const;
    void buildEditorTextFromDocument();

    QPlainTextEdit* m_editor = nullptr;
    QTimer* m_autoSaveTimer = nullptr;

    std::shared_ptr<Document> m_document;
    QString m_currentChapterId;
    int m_lastSavedRevision = 0;
};

#endif
