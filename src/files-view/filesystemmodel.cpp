#include "filesystemmodel.h"

#include <QFileInfo>
#include <QMimeDatabase>

FileSystemModel::FileSystemModel(QObject *parent) :
    QAbstractItemModel(parent)
{
}

FileSystemModel::~FileSystemModel()
{
    delete rootItem;
}

void FileSystemModel::setPaths(const QStringList &paths)
{
    beginResetModel();
    rootItem->removeChildren();
    foreach (const QString &path, paths)
        addPath(path.split("/", QString::SkipEmptyParts));
    endResetModel();
}

int FileSystemModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 3;
}

QVariant FileSystemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role) {
    case Qt::BackgroundColorRole:
    {
        FileSystemItem *item = static_cast<FileSystemItem*>(index.internalPointer());
        return item->backgroundColor();
    }
    case Qt::DisplayRole:
    {
        FileSystemItem *item = static_cast<FileSystemItem*>(index.internalPointer());
        return item->text(index.column());
    }
    case Qt::DecorationRole:
        if (index.column() == 0) {
            FileSystemItem *item = static_cast<FileSystemItem*>(index.internalPointer());
            return item->icon();
        }

        [[fallthrough]];
    default:
        return QVariant();
    }
}

Qt::ItemFlags FileSystemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;

    return QAbstractItemModel::flags(index);
}

QVariant FileSystemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->text(section);

    return QVariant();
}

QModelIndex FileSystemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    FileSystemItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<FileSystemItem*>(parent.internalPointer());

    FileSystemItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex FileSystemModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    FileSystemItem *childItem = static_cast<FileSystemItem*>(index.internalPointer());
    FileSystemItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int FileSystemModel::rowCount(const QModelIndex &parent) const
{
    FileSystemItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<FileSystemItem*>(parent.internalPointer());

    return parentItem->childCount();
}

void FileSystemModel::addPath(const QStringList &path)
{
    FileSystemItem *lastFound = rootItem;

    for (int partIndex = 0; partIndex < path.size(); ++partIndex) {
        // Search item in tree
        bool found = false;
        for (int i = 0; i < lastFound->childCount(); ++i) {
            FileSystemItem *child = lastFound->child(i);
            if (child->name() == path.at(partIndex)) {
                lastFound = child;
                found = true;
                break;
            }
        }

        // Add a new item
        if (!found) {
            // Get item path
            QString itemPath;
            for (int i = 0; i <= partIndex; ++i)
                itemPath.append("/" + path.at(i));

            auto *item = new FileSystemItem(itemPath, lastFound);
            lastFound = item;
        }
    }
}
