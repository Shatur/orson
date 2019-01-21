#ifndef TASK_H
#define TASK_H

#include "../packages-view/package.h"

#include <QIcon>

class Task
{
public:
    enum Category {
        Null = -1,
        InstallExplicity,
        InstallAsDepend,
        MarkExplicity,
        MarkAsDepend,
        Uninstall
    };

    Task();
    Task(const Package *package);
    ~Task();

    // Model-specific functions
    int row() const;
    Task *parent() const;

    Task *child(int row) const;
    int childCount() const;
    void addChild(Task *child);
    void removeChildren();

    // Item properties
    QString name() const;
    QIcon icon() const;

private:
    Task(Category category);

    Task *m_parent = nullptr;
    QVector<Task *> m_children;

    Category m_category = Null;
    QString m_name;
};

#endif // TASK_H
