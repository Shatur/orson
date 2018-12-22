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
    QTreeWidgetItem *lastFound = invisibleRootItem();
    foreach (const QString &pathPart, path.split("/", QString::SkipEmptyParts)) {
        // Search item in tree
        bool found = false;
        for (int i = 0; i < lastFound->childCount(); ++i) {
            QTreeWidgetItem *child = lastFound->child(i);
            if (child->text(0) == pathPart) {
                lastFound = child;
                found = true;
                break;
            }
        }

        if (!found) {
            // Add new item
            const QFileInfo itemInfo("/" + path.left(path.indexOf(pathPart) + pathPart.size()));
            QTreeWidgetItem *item = new QTreeWidgetItem;

            // Set information
            item->setText(0, pathPart);
            if (itemInfo.isFile()) {
                const QMimeDatabase mimeDatabase;
                const QMimeType type = mimeDatabase.mimeTypeForFile(itemInfo);
                item->setIcon(0, QIcon::fromTheme(type.iconName()));
                item->setText(1, QString::number(itemInfo.size()));
                item->setText(2, type.name());
                item->setData(0, Qt::UserRole, itemInfo.filePath());
            } else {
                item->setIcon(0, QIcon::fromTheme("folder"));
                item->setText(2, tr("Folder"));
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
