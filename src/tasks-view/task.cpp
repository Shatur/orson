#include "task.h"

#include "singleapplication.h"

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
        return SingleApplication::translate("TasksView", "Sync repositories") + " (-Sy)";
    case UpgradeAll:
        return SingleApplication::translate("TasksView", "Upgrade all") + " (-Su)";
    case InstallExplicity:
        return SingleApplication::translate("TasksView", "Install explicity") + " (-S)";
    case InstallAsDepend:
        return SingleApplication::translate("TasksView", "Install as depend") + " (-S --asdepend)";
    case Reinstall:
        return SingleApplication::translate("TasksView", "Reinstall") + " (-S)";
    case MarkAsExplicity:
        return SingleApplication::translate("TasksView", "Mark installed as explicity") + " (-D --asexplicit)";
    case MarkAsDepend:
        return SingleApplication::translate("TasksView", "Mark installed as depend") + " (-D --asdepend)";
    case Uninstall:
        return SingleApplication::translate("TasksView", "Uninstall") + " (-R)";
    case UninstallWithUnused:
        return SingleApplication::translate("TasksView", "Uninstall with unused dependencies") + " (-Rs)";
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
    case UninstallWithUnused:
        return QIcon::fromTheme("edit-paste-style");
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
