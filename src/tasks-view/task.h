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
    Category category() const;


private:
    explicit Task(Category category);

    Task *m_parent = nullptr;
    QVector<Task *> m_children;

    Category m_category = Null;
    QString m_name;
};

#endif // TASK_H
