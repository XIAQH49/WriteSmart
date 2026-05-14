#include "core/document/DocumentManager.h"
#include "core/document/Document.h"
#include "utils/Logger.h"

DocumentManager::DocumentManager(QObject* parent)
    : QObject(parent)
{
}

DocumentManager::~DocumentManager() = default;

std::shared_ptr<Document> DocumentManager::currentDocument() const
{
    return m_currentDocument;
}

void DocumentManager::setCurrentDocument(std::shared_ptr<Document> doc)
{
    m_currentDocument = std::move(doc);
}

std::shared_ptr<Document> DocumentManager::openDocument(const QString& filePath)
{
    m_currentDocument = std::make_shared<Document>();
    if (m_currentDocument->load(filePath)) {
        Logger::info("Document loaded: " + filePath);
        emit documentLoaded(m_currentDocument);
    }
    return m_currentDocument;
}

bool DocumentManager::saveDocument(const QString& filePath)
{
    if (!m_currentDocument) return false;
    QString path = filePath.isEmpty() ? m_currentDocument->filePath() : filePath;
    if (path.isEmpty()) return false;
    bool ok = m_currentDocument->save(path);
    if (ok) {
        emit documentSaved(path);
        emit modificationChanged(false);
    }
    return ok;
}

bool DocumentManager::closeDocument()
{
    if (!m_currentDocument) return true;
    m_currentDocument.reset();
    emit documentClosed();
    return true;
}

bool DocumentManager::isModified() const
{
    return m_currentDocument ? m_currentDocument->isModified() : false;
}
