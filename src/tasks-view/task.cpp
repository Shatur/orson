#include "task.h"

Task::Task(const Package *package) :
    m_name(package->name()),
    m_icon(package->icon())
{
}

// Create root item with categories
Task::Task()
{
    addChild(new Task(UpgradeAll));
    addChild(new Task(InstallExplicity));
    addChild(new Task(InstallAsDepend));
    addChild(new Task(Reinstall));
    addChild(new Task(MarkAsExplicity));
    addChild(new Task(MarkAsDepend));
    addChild(new Task(Uninstall));
}

// Create category item
Task::Task(Task::Category category) :
    m_category(category)
{
}

QVector<Task *> Task::children() const
{
    return m_children;
}

Task::Category Task::categoryType() const
{
    return m_category;
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

void Task::addChild(Task *child)
{
    child->m_parent = this;
    m_children.append(child);
}

void Task::removeChild(Task *child)
{
    child->m_parent = nullptr;
    m_children.remove(m_children.indexOf(child));
}

void Task::removeChildren()
{
    qDeleteAll(m_children);
    m_children.clear();
}

QString Task::name() const
{
    switch (m_category) {
    case UpgradeAll:
        return "Upgrade all";
    case InstallExplicity:
        return "Install explicity";
    case InstallAsDepend:
        return "Install as depend";
    case Reinstall:
        return "Reinstall";
    case MarkAsExplicity:
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
    case UpgradeAll:
        return QIcon::fromTheme("go-up");
    case InstallExplicity:
        return QIcon::fromTheme("edit-download");
    case InstallAsDepend:
        return QIcon::fromTheme("edit-add");
    case Reinstall:
        return QIcon::fromTheme("reload");
    case MarkAsExplicity:
        return QIcon::fromTheme("exchange-positions");
    case MarkAsDepend:
        return QIcon::fromTheme("exchange-positions-clockwise");
    case Uninstall:
        return QIcon::fromTheme("edit-delete");
    default:
        return m_icon;
    }
}
