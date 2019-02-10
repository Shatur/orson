#ifndef TASK_H
#define TASK_H

#include <QIcon>

class Package;

class Task
{
public:
    enum Type {
        Item = -1,
        Sync,
        UpgradeAll,
        InstallExplicity,
        InstallAsDepend,
        Reinstall,
        MarkAsExplicity,
        MarkAsDepend,
        Uninstall
    };

    explicit Task(Package *m_package);
    explicit Task(Type type);
    Task() = default;
    ~Task();

    // Model-specific functions
    int row() const;
    Task *parent() const;

    QVector<Task *> children() const;
    void addChild(Task *child);
    void removeChild(Task *child);
    void removeChildren();

    // Item properties
    Type type() const;
    Package *package() const;

    static QString categoryName(Type category);
    static QIcon categoryIcon(Type category);

private:
    Task *m_parent = nullptr;
    QVector<Task *> m_children;

    Type m_category = Item;
    Package *m_package = nullptr;
};

#endif // TASK_H
