#ifndef FILE_H
#define FILE_H

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

    QVector<File *> children() const;
    void addChild(File *child);
    void removeChildren();

    // Item properties
    QString name() const;
    QIcon icon() const;
    QString path() const;
    bool isFile() const;
    bool isReadable() const;
    bool isMissing() const;

private:
    File *m_parent = nullptr;
    QVector<File *> m_children;

    QString m_nameColumn;
    QString m_sizeColumn;
    QString m_typeColumn;
    QFileInfo m_info;
    QIcon m_icon;
    bool m_missing = false;
};

#endif // FILE_H
