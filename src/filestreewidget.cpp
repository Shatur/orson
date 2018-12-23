#include "filestreewidget.h"

#include <QFileInfo>
#include <QMimeDatabase>
#include <QDesktopServices>
#include <QUrl>

FilesTreeWidget::FilesTreeWidget(QWidget *parent) :
    QTreeWidget(parent)
{
    connect(this, &FilesTreeWidget::itemDoubleClicked, this, &FilesTreeWidget::open);
}

void FilesTreeWidget::addPath(const QString &path)
{
    const QStringList pathParts = path.split("/", QString::SkipEmptyParts);
    QTreeWidgetItem *lastFound = invisibleRootItem();

    for (int i = 0; i < pathParts.size(); ++i) {
        // Search item in tree
        bool found = false;
        for (int j = 0; j < lastFound->childCount(); ++j) {
            QTreeWidgetItem *child = lastFound->child(j);
            if (child->text(0) == pathParts.at(i)) {
                lastFound = child;
                found = true;
                break;
            }
        }

        // Add new item
        if (!found) {
            // Get item path
            QString itemPath;
            for (int j = 0; j <= i; ++j)
                itemPath.append("/" + pathParts.at(j));

            const QFileInfo itemInfo(itemPath);
            QTreeWidgetItem *item = new QTreeWidgetItem;

            // Set information
            item->setText(0, pathParts.at(i));
            if (itemInfo.isFile()) {
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

void FilesTreeWidget::open(QTreeWidgetItem *item)
{
    QDesktopServices::openUrl(item->data(0, Qt::UserRole).value<QUrl>());
}
