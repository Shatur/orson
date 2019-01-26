#ifndef TASK_H
#define TASK_H

#include "../packages-view/package.h"

class Task
{
public:
    enum Category {
        Null = -1,
        InstallExplicity,
        InstallAsDepend,
        Reinstall,
        MarkAsExplicity,
        MarkAsDepend,
        Uninstall
    };

    explicit Task(const Package *package);
    Task();
    ~Task();

    // Model-specific functions
    int row() const;
    Task *parent() const;

    QVector<Task *> children() const;
    void addChild(Task *child);
    void removeChild(Task *child);
    void removeChildren();

    // Item properties
    QString name() const;
    QIcon icon() const;
    Category categoryType() const;


private:
    explicit Task(Category categoryType);

    Task *m_parent = nullptr;
    QVector<Task *> m_children;

    Category m_category = Null;
    QString m_name;
    QIcon m_icon;
};

#endif // TASK_H
