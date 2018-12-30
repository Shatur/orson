#ifndef FILESYSTEMITEM_H
#define FILESYSTEMITEM_H

#include <QList>
#include <QVariant>
#include <QIcon>
#include <QFileInfo>

class FileSystemItem
{
public:
    FileSystemItem();
    FileSystemItem(const QString &path, FileSystemItem *parent);
    ~FileSystemItem();

    // Model-specific functions
    int row() const;
    const QString &text(int column) const;

    FileSystemItem *child(int row);
    void addChild(FileSystemItem *child);
    void removeChildren();
    int childCount() const;

    FileSystemItem *parent() const;
    void setParent(FileSystemItem *parent);

    // Item properties
    const QString &name() const;
    const QIcon &icon() const;
    const QColor &backgroundColor() const;
    QString path() const;
    bool isFile() const;
    bool isReadable() const;

private:
    FileSystemItem *m_parent;
    QList<FileSystemItem*> m_children;

    QString m_nameColumn;
    QString m_sizeColumn;
    QString m_typeColumn;
    QFileInfo m_info;
    QIcon m_icon;
    QColor m_backgroundColor = {0, 0, 0, 0};
};

#endif // FILESYSTEMITEM_H
