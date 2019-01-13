#ifndef FILESYSTEMITEM_H
#define FILESYSTEMITEM_H

#include <QIcon>
#include <QFileInfo>

class File
{
public:
    File();
    File(const QString &path, File *parent);
    ~File();

    // Model-specific functions
    int row() const;
    QString text(int column) const;
    File *parent() const;

    File *child(int row) const;
    int childCount() const;
    void addChild(File *child);
    void removeChildren();

    // Item properties
    QString name() const;
    QIcon icon() const;
    QColor backgroundColor() const;
    QString path() const;
    bool isFile() const;
    bool isReadable() const;

private:
    File *m_parent = nullptr;
    QVector<File *> m_children;

    QString m_nameColumn;
    QString m_sizeColumn;
    QString m_typeColumn;
    QFileInfo m_info;
    QIcon m_icon;
    QColor m_backgroundColor = {0, 0, 0, 0};
};

#endif // FILESYSTEMITEM_H
