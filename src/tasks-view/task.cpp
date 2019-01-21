#include "task.h"

// Create root item with categories
Task::Task()
{
    addChild(new Task(InstallExplicity));
    addChild(new Task(InstallAsDepend));
    addChild(new Task(MarkExplicity));
    addChild(new Task(MarkAsDepend));
    addChild(new Task(Uninstall));
}

// Create category item
Task::Task(Task::Category category) :
    m_category(category)
{
}

Task::Task(const Package *package)
{
    m_name = package->name();
}

Task::~Task()
{
    qDeleteAll(m_children);
}

int Task::row() const
{
    if (m_parent != nullptr)
        return m_parent->m_children.indexOf(const_cast<Task *>(this));

    return m_category;
}

Task *Task::parent() const
{
    return m_parent;
}

Task *Task::child(int row) const
{
    return m_children.value(row);
}

int Task::childCount() const
{
    return m_children.count();
}

void Task::addChild(Task *item)
{
    item->m_parent = this;
    m_children.append(item);
}

void Task::removeChildren()
{
    qDeleteAll(m_children);
    m_children.clear();
}

QString Task::name() const
{
    switch (m_category) {
    case InstallExplicity:
        return "Install explicity";
    case InstallAsDepend:
        return "Install as depend";
    case MarkExplicity:
        return "Mark installed as explicity";
    case MarkAsDepend:
        return "Mark installed as depend";
    case Uninstall:
        return "Uninstall";
    default:
        return m_name;
    }
}

QIcon Task::icon() const
{
    switch (m_category) {
    case InstallExplicity:
        return QIcon::fromTheme("edit-add");
    case InstallAsDepend:
        return QIcon::fromTheme("format-add-node");
    case MarkExplicity:
        return QIcon::fromTheme("exchange-positions");
    case MarkAsDepend:
        return QIcon::fromTheme("exchange-positions-clockwise");
    case Uninstall:
        return QIcon::fromTheme("edit-delete");
    default:
        return QIcon::fromTheme(m_name);
    }
}
