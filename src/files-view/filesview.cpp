#include "filesview.h"
#include "singleapplication.h"

#include <QDesktopServices>
#include <QContextMenuEvent>
#include <QClipboard>
#include <QMimeData>
#include <QHeaderView>

FilesView::FilesView(QWidget *parent) :
    QTreeView(parent)
{
    setModel(new FilesModel(this));
    header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // Setup context menu
    m_menu = new QMenu(this);
    m_menu->addAction(QIcon::fromTheme("document-open"), tr("Open"), this, &FilesView::open);
    m_menu->addAction(QIcon::fromTheme("folder"), tr("Open if file manager"), this, &FilesView::openInFileManager);
    m_menu->addAction(QIcon::fromTheme("edit-copy"), tr("Copy"), this, &FilesView::copyFile);
    m_menu->addAction(QIcon::fromTheme("edit-copy"), tr("Copy name"), this, &FilesView::copyName);
    m_menu->addAction(QIcon::fromTheme("edit-copy"), tr("Copy path"), this, &FilesView::copyPath);
}

FilesModel *FilesView::model() const
{
    return qobject_cast<FilesModel *>(QTreeView::model());
}

void FilesView::open() const
{
    const auto *item = static_cast<File *>(currentIndex().internalPointer());
    QDesktopServices::openUrl(item->path());
}

void FilesView::openInFileManager() const
{
    const auto *item = static_cast<File *>(currentIndex().internalPointer());
    QDesktopServices::openUrl(item->parent()->path());
}

void FilesView::copyFile() const
{
    const auto *item = static_cast<File *>(currentIndex().internalPointer());
    auto *mimeData = new QMimeData;
    mimeData->setUrls({QUrl::fromLocalFile(item->path())});

    SingleApplication::clipboard()->setMimeData(mimeData);
}

void FilesView::copyName() const
{
    const auto *item = static_cast<File *>(currentIndex().internalPointer());
    SingleApplication::clipboard()->setText(item->name());
}

void FilesView::copyPath() const
{
    const auto *item = static_cast<File *>(currentIndex().internalPointer());
    SingleApplication::clipboard()->setText(item->path());
}

void FilesView::contextMenuEvent(QContextMenuEvent *event)
{
    const auto *item = static_cast<File *>(indexAt(event->pos()).internalPointer());
    if (item != nullptr) {
        // Enable "Copy" only for files
        if (item->isFile())
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
