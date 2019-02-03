#include "task.h"

Task::Task(Package *package) :
    m_package(package)
{
}

// Create category item
Task::Task(Task::Type category) :
    m_category(category)
{
}

Task::~Task()
{
    qDeleteAll(m_children);
}

QVector<Task *> Task::children() const
{
    return m_children;
}

Task::Type Task::type() const
{
    return m_category;
}

Package *Task::package() const
{
    return m_package;
}

QString Task::categoryName(Task::Type category)
{
    switch (category) {
    case Sync:
        return "Sync repositories";
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
        return QString();
    }
}

QIcon Task::categoryIcon(Task::Type category)
{
    switch (category) {
    case Sync:
        return QIcon::fromTheme("mail-send-receive");
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
        return QIcon();
    }
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
    delete child;
}

void Task::removeChildren()
{
    qDeleteAll(m_children);
    m_children.clear();
}
