#include "filesmodel.h"

FilesModel::FilesModel(QObject *parent) :
    QAbstractItemModel(parent)
{
}

FilesModel::~FilesModel()
{
    delete rootItem;
}

QVariant FilesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role) {
    case Qt::BackgroundColorRole:
    {
        File *item = static_cast<File *>(index.internalPointer());
        return item->backgroundColor();
    }
    case Qt::DisplayRole:
    {
        File *item = static_cast<File*>(index.internalPointer());
        return item->text(index.column());
    }
    case Qt::DecorationRole:
        if (index.column() == 0) {
            File *item = static_cast<File *>(index.internalPointer());
            return item->icon();
        }

        [[fallthrough]];
    default:
        return QVariant();
    }
}

QVariant FilesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->text(section);

    return QVariant();
}

QModelIndex FilesModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    File *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<File *>(parent.internalPointer());

    File *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex FilesModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    File *childItem = static_cast<File *>(index.internalPointer());
    File *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int FilesModel::rowCount(const QModelIndex &parent) const
{
    File *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<File *>(parent.internalPointer());

    return parentItem->childCount();
}

int FilesModel::columnCount(const QModelIndex &) const
{
    return 3;
}

void FilesModel::setPaths(const QStringList &paths)
{
    beginResetModel();
    rootItem->removeChildren();
    foreach (const QString &path, paths)
        addPath(path.split("/", QString::SkipEmptyParts));
    endResetModel();
}

void FilesModel::addPath(const QStringList &path)
{
    File *lastFound = rootItem;

    for (int partIndex = 0; partIndex < path.size(); ++partIndex) {
        // Search item in tree
        bool found = false;
        for (int i = 0; i < lastFound->childCount(); ++i) {
            File *child = lastFound->child(i);
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

            auto *item = new File(itemPath, lastFound);
            lastFound = item;
        }
    }
}
