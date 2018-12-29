#include "filestreewidget.h"

#include <QMimeData>
#include <QGuiApplication>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QDesktopServices>
#include <QContextMenuEvent>
#include <QUrl>
#include <QClipboard>

FilesTreeWidget::FilesTreeWidget(QWidget *parent) :
    QTreeWidget(parent)
{
    // Double-click to open files
    connect(this, &FilesTreeWidget::itemDoubleClicked, this, &FilesTreeWidget::open);

    // Setup context menu
    menu.addAction(QIcon::fromTheme("document-open"), tr("Open"), this, &FilesTreeWidget::open);
    menu.addAction(QIcon::fromTheme("folder"), tr("Open if file manager"), this, &FilesTreeWidget::openInFileManager);
    menu.addAction(QIcon::fromTheme("edit-copy"), tr("Copy"), this, &FilesTreeWidget::copyFile);
    menu.addAction(QIcon::fromTheme("edit-copy"), tr("Copy name"), this, &FilesTreeWidget::copyName);
    menu.addAction(QIcon::fromTheme("edit-copy"), tr("Copy path"), this, &FilesTreeWidget::copyPath);
}

void FilesTreeWidget::addPath(const QString &path)
{
    const QStringList pathParts = path.split("/", QString::SkipEmptyParts);
    QTreeWidgetItem *lastFound = invisibleRootItem();

    for (int partIndex = 0; partIndex < pathParts.size(); ++partIndex) {
        // Search item in tree
        bool found = false;
        for (int i = 0; i < lastFound->childCount(); ++i) {
            QTreeWidgetItem *child = lastFound->child(i);
            if (child->text(0) == pathParts.at(partIndex)) {
                lastFound = child;
                found = true;
                break;
            }
        }

        // Add new item
        if (!found) {
            // Get item path
            QString itemPath;
            for (int i = 0; i <= partIndex; ++i)
                itemPath.append("/" + pathParts.at(i));

            const QFileInfo itemInfo(itemPath);
            auto *item = new QTreeWidgetItem;

            // Set information
            item->setText(0, pathParts.at(partIndex));
            if (!itemInfo.exists()) {
                item->setText(2, tr("Missing"));
                item->setIcon(0, QIcon::fromTheme("dialog-error"));
                for (int column = 0; column < item->columnCount(); ++column)
                    item->setBackgroundColor(column, QColor(255, 0, 0, 127));
            } else if (itemInfo.isFile()) {
                // Set icon
                const QMimeDatabase mimeDatabase;
                const QMimeType type = mimeDatabase.mimeTypeForFile(itemInfo);
                if (itemInfo.isReadable())
                    item->setIcon(0, QIcon::fromTheme(type.iconName()));
                else
                    item->setIcon(0, QIcon::fromTheme("lock"));

                item->setText(1, QString::number(itemInfo.size()));
                item->setText(2, type.name());
                item->setData(0, Qt::UserRole, itemInfo.filePath());
            } else if (itemInfo.isDir()) {
                // Set icon
                if (itemInfo.isReadable())
                    item->setIcon(0, QIcon::fromTheme("folder"));
                else
                    item->setIcon(0, QIcon::fromTheme("lock"));

                item->setText(2, tr("Folder"));
                item->setData(0, Qt::UserRole, itemInfo.filePath());
            } else {
                // No access to read any information
                item->setIcon(0, QIcon::fromTheme("lock"));
                item->setText(2, tr("No access"));
                item->setData(0, Qt::UserRole, itemInfo.filePath());
            }

            lastFound->addChild(item);
            lastFound = item;
        }
    }
}

void FilesTreeWidget::open()
{
    QTreeWidgetItem *item = currentItem();
    QDesktopServices::openUrl(item->data(0, Qt::UserRole).toUrl());
}

void FilesTreeWidget::openInFileManager()
{
    QTreeWidgetItem *item = currentItem();
    QDesktopServices::openUrl(item->parent()->data(0, Qt::UserRole).toUrl());
}

void FilesTreeWidget::copyFile()
{
    QTreeWidgetItem *item = currentItem();
    QMimeData* mimeData = new QMimeData();
    mimeData->setUrls({QUrl::fromLocalFile(item->data(0, Qt::UserRole).toString())});

    QGuiApplication::clipboard()->setMimeData(mimeData);
}

void FilesTreeWidget::copyName()
{
    QTreeWidgetItem *item = currentItem();
    QGuiApplication::clipboard()->setText(item->text(0));
}

void FilesTreeWidget::copyPath()
{
    QTreeWidgetItem *item = currentItem();
    QGuiApplication::clipboard()->setText(item->data(0, Qt::UserRole).toString());
}

void FilesTreeWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QTreeWidgetItem *item = itemAt(event->pos());
    if (item != nullptr) {
        // Hide "Copy" for folders
        if (item->icon(0).name() == "folder")
            menu.actions().at(2)->setEnabled(false);
        else
            menu.actions().at(2)->setEnabled(true);

        menu.exec(event->globalPos());
    }
}
