#include "filesview.h"
#include "singleapplication.h"
#include "filesmodel.h"
#include "file.h"

#include <QDesktopServices>
#include <QContextMenuEvent>
#include <QClipboard>
#include <QMimeData>
#include <QHeaderView>
#include <QMenu>

FilesView::FilesView(QWidget *parent) :
    QTreeView(parent)
{
    setModel(new FilesModel(this));
    header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // Setup context menu
    m_menu = new QMenu(this);
    m_menu->addAction(QIcon::fromTheme("document-open"), tr("Open"), this, &FilesView::openCurrent);
    m_menu->addAction(QIcon::fromTheme("folder"), tr("Open if file manager"), this, &FilesView::openCurrentInFilesystem);
    m_menu->addAction(QIcon::fromTheme("edit-copy"), tr("Copy"), this, &FilesView::copyCurrentFile);
    m_menu->addAction(QIcon::fromTheme("edit-copy"), tr("Copy name"), this, &FilesView::copyCurrentName);
    m_menu->addAction(QIcon::fromTheme("edit-copy"), tr("Copy path"), this, &FilesView::copyCurrentPath);
}

FilesModel *FilesView::model() const
{
    return qobject_cast<FilesModel *>(QTreeView::model());
}

void FilesView::openCurrent() const
{
    const auto *file = static_cast<File *>(currentIndex().internalPointer());
    QDesktopServices::openUrl(file->path());
}

void FilesView::openCurrentInFilesystem() const
{
    const auto *file = static_cast<File *>(currentIndex().internalPointer());
    QDesktopServices::openUrl(file->parent()->path());
}

void FilesView::copyCurrentFile() const
{
    const auto *file = static_cast<File *>(currentIndex().internalPointer());
    auto *mimeData = new QMimeData;
    mimeData->setUrls({QUrl::fromLocalFile(file->path())});

    SingleApplication::clipboard()->setMimeData(mimeData);
}

void FilesView::copyCurrentName() const
{
    const auto *file = static_cast<File *>(currentIndex().internalPointer());
    SingleApplication::clipboard()->setText(file->name());
}

void FilesView::copyCurrentPath() const
{
    const auto *file = static_cast<File *>(currentIndex().internalPointer());
    SingleApplication::clipboard()->setText(file->path());
}

void FilesView::contextMenuEvent(QContextMenuEvent *event)
{
    const auto *file = static_cast<File *>(indexAt(event->pos()).internalPointer());
    if (file != nullptr) {
        // Enable "Copy" only for files
        if (file->isFile())
            m_menu->actions().at(2)->setEnabled(true);
        else
            m_menu->actions().at(2)->setEnabled(false);

        m_menu->exec(event->globalPos());
    }
}

void FilesView::mouseDoubleClickEvent(QMouseEvent *event)
{
    const QModelIndex index = indexAt(event->pos());
    if (index.isValid()) {
        const auto *item = static_cast<File *>(index.internalPointer());
        QDesktopServices::openUrl(item->path());
    }
}

void FilesView::setModel(QAbstractItemModel *model)
{
    QTreeView::setModel(model);
}
