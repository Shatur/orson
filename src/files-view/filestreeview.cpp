#include "filestreeview.h"
#include "singleapplication.h"

#include <QMimeData>
#include <QDesktopServices>
#include <QContextMenuEvent>
#include <QClipboard>
#include <QHeaderView>

FilesTreeView::FilesTreeView(QWidget *parent) :
    QTreeView(parent)
{
    setModel(m_model);
    header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // Setup context menu
    m_menu.addAction(QIcon::fromTheme("document-open"), tr("Open"), this, &FilesTreeView::open);
    m_menu.addAction(QIcon::fromTheme("folder"), tr("Open if file manager"), this, &FilesTreeView::openInFileManager);
    m_menu.addAction(QIcon::fromTheme("edit-copy"), tr("Copy"), this, &FilesTreeView::copyFile);
    m_menu.addAction(QIcon::fromTheme("edit-copy"), tr("Copy name"), this, &FilesTreeView::copyName);
    m_menu.addAction(QIcon::fromTheme("edit-copy"), tr("Copy path"), this, &FilesTreeView::copyPath);
}

FilesModel *FilesTreeView::model() const
{
    return m_model;
}

void FilesTreeView::open() const
{
    const auto item = static_cast<File *>(currentIndex().internalPointer());
    QDesktopServices::openUrl(item->path());
}

void FilesTreeView::openInFileManager() const
{
    const auto item = static_cast<File *>(currentIndex().internalPointer());
    QDesktopServices::openUrl(item->parent()->path());
}

void FilesTreeView::copyFile() const
{
    const auto item = static_cast<File *>(currentIndex().internalPointer());
    QMimeData* mimeData = new QMimeData();
    mimeData->setUrls({QUrl::fromLocalFile(item->path())});

    SingleApplication::clipboard()->setMimeData(mimeData);
}

void FilesTreeView::copyName() const
{
    const auto item = static_cast<File *>(currentIndex().internalPointer());
    SingleApplication::clipboard()->setText(item->name());
}

void FilesTreeView::copyPath() const
{
    const auto item = static_cast<File *>(currentIndex().internalPointer());
    SingleApplication::clipboard()->setText(item->path());
}

void FilesTreeView::contextMenuEvent(QContextMenuEvent *event)
{
    const auto item = static_cast<File *>(indexAt(event->pos()).internalPointer());
    if (item != nullptr) {
        // Enable "Copy" only for files
        if (item->isFile())
            m_menu.actions().at(2)->setEnabled(true);
        else
            m_menu.actions().at(2)->setEnabled(false);

        m_menu.exec(event->globalPos());
    }
}

void FilesTreeView::mouseDoubleClickEvent(QMouseEvent *event)
{
    const QModelIndex index = indexAt(event->pos());
    if (index.isValid()) {
        const auto item = static_cast<File *>(index.internalPointer());
        QDesktopServices::openUrl(item->path());
    }
}
