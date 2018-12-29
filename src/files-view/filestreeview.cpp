#include "filestreeview.h"
#include "filesystemitem.h"

#include <QMimeData>
#include <QGuiApplication>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QDesktopServices>
#include <QContextMenuEvent>
#include <QUrl>
#include <QClipboard>

FilesTreeView::FilesTreeView(QWidget *parent) :
    QTreeView(parent)
{
    // Setup context menu
    menu.addAction(QIcon::fromTheme("document-open"), tr("Open"), this, &FilesTreeView::open);
    menu.addAction(QIcon::fromTheme("folder"), tr("Open if file manager"), this, &FilesTreeView::openInFileManager);
    menu.addAction(QIcon::fromTheme("edit-copy"), tr("Copy"), this, &FilesTreeView::copyFile);
    menu.addAction(QIcon::fromTheme("edit-copy"), tr("Copy name"), this, &FilesTreeView::copyName);
    menu.addAction(QIcon::fromTheme("edit-copy"), tr("Copy path"), this, &FilesTreeView::copyPath);
}

void FilesTreeView::open() const
{
    const auto item = static_cast<FileSystemItem *>(currentIndex().internalPointer());
    QDesktopServices::openUrl(item->path());
}

void FilesTreeView::openInFileManager() const
{
    const auto item = static_cast<FileSystemItem *>(currentIndex().internalPointer());
    QDesktopServices::openUrl(item->parent()->path());
}

void FilesTreeView::copyFile() const
{
    const auto item = static_cast<FileSystemItem *>(currentIndex().internalPointer());
    QMimeData* mimeData = new QMimeData();
    mimeData->setUrls({QUrl::fromLocalFile(item->path())});

    QGuiApplication::clipboard()->setMimeData(mimeData);
}

void FilesTreeView::copyName() const
{
    const auto item = static_cast<FileSystemItem *>(currentIndex().internalPointer());
    QGuiApplication::clipboard()->setText(item->name());
}

void FilesTreeView::copyPath() const
{
    const auto item = static_cast<FileSystemItem *>(currentIndex().internalPointer());
    QGuiApplication::clipboard()->setText(item->path());
}

void FilesTreeView::contextMenuEvent(QContextMenuEvent *event)
{
    const auto item = static_cast<FileSystemItem *>(indexAt(event->pos()).internalPointer());
    if (item != nullptr) {
        // Enable "Copy" only for files
        if (item->isFile())
            menu.actions().at(2)->setEnabled(true);
        else
            menu.actions().at(2)->setEnabled(false);

        menu.exec(event->globalPos());
    }
}

void FilesTreeView::mouseDoubleClickEvent(QMouseEvent *event)
{
    const QModelIndex index = indexAt(event->pos());
    if (index.isValid()) {
        const auto item = static_cast<FileSystemItem *>(index.internalPointer());
        QDesktopServices::openUrl(item->path());
    }
}
