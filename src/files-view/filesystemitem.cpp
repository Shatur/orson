#include <QStringList>
#include <QMimeDatabase>

#include "filesystemitem.h"

FileSystemItem::FileSystemItem()
{
    m_parent = nullptr;
    m_nameColumn = "Name";
    m_sizeColumn = "Size";
    m_typeColumn = "Type";
    m_info.setFile("/");
}

FileSystemItem::FileSystemItem(const QString &path, FileSystemItem *parent) :
    m_info(path)
{
    m_nameColumn = m_info.fileName();
    if (!m_info.exists() && parent->isReadable()) {
        m_icon = QIcon::fromTheme("dialog-error");
        m_typeColumn = "Missing";
        m_backgroundColor = {255, 0, 0, 127};
    } else if (m_info.isFile()) {
        const QMimeDatabase mimeDatabase;
        const QMimeType type = mimeDatabase.mimeTypeForFile(m_info);

        if (m_info.isReadable())
            m_icon = QIcon::fromTheme(type.iconName());
        else
            m_icon = QIcon::fromTheme("lock");

        m_sizeColumn = QString::number(m_info.size());
        m_typeColumn = type.name();
    } else if (m_info.isDir()) {
        if (m_info.isReadable())
            m_icon = QIcon::fromTheme("folder");
        else
            m_icon = QIcon::fromTheme("lock");

        m_typeColumn = "Folder";
    } else {
        // No access to read any information
        m_typeColumn = "No access";
        m_icon = QIcon::fromTheme("lock");
    }

    parent->addChild(this);
}

FileSystemItem::~FileSystemItem()
{
    qDeleteAll(m_children);
}

int FileSystemItem::row() const
{
    if (m_parent)
        return m_parent->m_children.indexOf(const_cast<FileSystemItem*>(this));

    return 0;
}

QString FileSystemItem::text(int column) const
{
    switch (column) {
    case 0:
        return m_nameColumn;
    case 1:
        return m_sizeColumn;
    case 2:
        return m_typeColumn;
    default:
        qFatal("Unknown column");
    }
}

FileSystemItem *FileSystemItem::child(int row)
{
    return m_children.value(row);
}

void FileSystemItem::addChild(FileSystemItem *item)
{
    item->setParent(this);
    m_children.append(item);
}

void FileSystemItem::removeChildren()
{
    qDeleteAll(m_children);
    m_children.clear();
}

int FileSystemItem::childCount() const
{
    return m_children.count();
}

FileSystemItem *FileSystemItem::parent() const
{
    return m_parent;
}

void FileSystemItem::setParent(FileSystemItem *parentItem)
{
    m_parent = parentItem;
}

QString FileSystemItem::name() const
{
    return m_nameColumn;
}

QIcon FileSystemItem::icon() const
{
    return m_icon;
}

QColor FileSystemItem::backgroundColor() const
{
    return m_backgroundColor;
}

QString FileSystemItem::path() const
{
    return m_info.filePath();
}

bool FileSystemItem::isFile() const
{
    return m_info.isFile();
}

bool FileSystemItem::isReadable() const
{
    return m_info.isReadable();
}
