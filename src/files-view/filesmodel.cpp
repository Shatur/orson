#include "filesmodel.h"

FilesModel::FilesModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    m_rootItem = new File;
}

FilesModel::~FilesModel()
{
    delete m_rootItem;
}

QVariant FilesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    auto *item = static_cast<File *>(index.internalPointer());

    switch (role) {
    case Qt::BackgroundColorRole:
        return item->backgroundColor();
    case Qt::DisplayRole:
        return item->text(index.column());
    case Qt::DecorationRole:
        if (index.column() == 0)
            return item->icon();

        [[fallthrough]];
    default:
        return QVariant();
    }
}

QVariant FilesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return m_rootItem->text(section);

    return QVariant();
}

QModelIndex FilesModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    File *parentItem;
    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<File *>(parent.internalPointer());

    File *childItem = parentItem->children().at(row);
    if (!childItem)
        return QModelIndex();

    return createIndex(row, column, childItem);
}

QModelIndex FilesModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    auto *childItem = static_cast<File *>(index.internalPointer());
    File *parentItem = childItem->parent();

    if (parentItem == m_rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int FilesModel::rowCount(const QModelIndex &parent) const
{
    File *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<File *>(parent.internalPointer());

    return parentItem->children().size();
}

int FilesModel::columnCount(const QModelIndex &) const
{
    return 3;
}

void FilesModel::setPaths(const QStringList &paths)
{
    beginResetModel();
    m_rootItem->removeChildren();
    foreach (const QString &path, paths)
        addPath(path.split("/", QString::SkipEmptyParts));
    endResetModel();
}

void FilesModel::addPath(const QStringList &path)
{
    File *lastFound = m_rootItem;

    for (int partIndex = 0; partIndex < path.size(); ++partIndex) {
        // Search item in tree
        bool found = false;
        foreach (File *child, lastFound->children()) {
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
