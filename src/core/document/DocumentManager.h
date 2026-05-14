#ifndef WRITESMART_DOCUMENT_MANAGER_H
#define WRITESMART_DOCUMENT_MANAGER_H

#include <QObject>
#include <memory>

class Document;

class DocumentManager : public QObject {
    Q_OBJECT

public:
    explicit DocumentManager(QObject* parent = nullptr);
    ~DocumentManager() override;

    std::shared_ptr<Document> currentDocument() const;
    void setCurrentDocument(std::shared_ptr<Document> doc);

    std::shared_ptr<Document> openDocument(const QString& filePath);
    bool saveDocument(const QString& filePath = QString());
    bool closeDocument();

    bool isModified() const;

signals:
    void documentLoaded(std::shared_ptr<Document> doc);
    void documentSaved(const QString& filePath);
    void documentClosed();
    void modificationChanged(bool modified);

private:
    std::shared_ptr<Document> m_currentDocument;
};

#endif // WRITESMART_DOCUMENT_MANAGER_H
