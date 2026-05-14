#ifndef WRITESMART_MARKDOWN_EDITOR_H
#define WRITESMART_MARKDOWN_EDITOR_H

#include <QPlainTextEdit>

class MarkdownEditor : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit MarkdownEditor(QWidget* parent = nullptr);

    int lineNumberAreaWidth() const;
    void lineNumberAreaPaintEvent(QPaintEvent* event);

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void updateLineNumberAreaWidth(int blockCount);
    void updateLineNumberArea(const QRect& rect, int dy);
    void highlightCurrentLine();

private:
    QWidget* m_lineNumberArea = nullptr;
};

class LineNumberArea : public QWidget {
public:
    explicit LineNumberArea(MarkdownEditor* editor);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    MarkdownEditor* m_editor = nullptr;
};

#endif // WRITESMART_MARKDOWN_EDITOR_H
