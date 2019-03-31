#include "packagesview.h"
#include "packagesmodel.h"
#include "package.h"
#include "../appsettings.h"
#include "../tasks-view/task.h"

#include <QHeaderView>
#include <QContextMenuEvent>
#include <QMenu>

PackagesView::PackagesView(QWidget *parent) :
    QTreeView(parent)
{
    // Setup menu
    m_menu = new QMenu(this);
    m_installExplicityAction = m_menu->addAction(Task::categoryIcon(Task::InstallExplicity), Task::categoryName(Task::InstallExplicity));
    m_installAsDependAction = m_menu->addAction(Task::categoryIcon(Task::InstallAsDepend), Task::categoryName(Task::InstallAsDepend));
    m_reinstallAction = m_menu->addAction(Task::categoryIcon(Task::Reinstall), Task::categoryName(Task::Reinstall));
    m_markAsExplicityAction = m_menu->addAction(Task::categoryIcon(Task::MarkAsExplicity), Task::categoryName(Task::MarkAsExplicity));
    m_markAsDependAction = m_menu->addAction(Task::categoryIcon(Task::MarkAsDepend), Task::categoryName(Task::MarkAsDepend));
    m_uninstallAction = m_menu->addAction(Task::categoryIcon(Task::Uninstall), Task::categoryName(Task::Uninstall));
    m_uninstallWithUnusedAction = m_menu->addAction(Task::categoryIcon(Task::UninstallWithUnused), Task::categoryName(Task::UninstallWithUnused));
    m_menu->addSeparator();
    m_syncAction = m_menu->addAction(Task::categoryIcon(Task::Sync), Task::categoryName(Task::Sync));
    m_upgradeAllAction = m_menu->addAction(Task::categoryIcon(Task::UpgradeAll), Task::categoryName(Task::UpgradeAll));
    connect(m_menu, &QMenu::triggered, this, &PackagesView::processMenuAction);

    // Setup items
    sortByColumn(-1, Qt::AscendingOrder); // Show item unsorted by default
    setModel(new PackagesModel(this));
    header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    connect(selectionModel(), &QItemSelectionModel::currentChanged, this, &PackagesView::processSelectionChanging);
    connect(model(), &PackagesModel::modelAboutToBeReset, this, &PackagesView::clearAllOperations);

    // Emit current package changed signal on data change
    connect(model(), &PackagesModel::packageChanged, [&](Package *package) {
        if (package == currentPackage())
            emit currentPackageChanged(package);
    });
}

void PackagesView::search(const QString &text, PackagesView::SearchType type)
{
    // Search packages in AUR
    if (model()->mode() == PackagesModel::AUR) {
        switch (type) {
        case NameDescription:
            model()->aurQuery(text, "name-desc");
            return;
        case Name:
            model()->aurQuery(text, "name");
            return;
        case Maintainer:
            model()->aurQuery(text, "maintainer");
            return;
        default:
            qFatal("Unsupported search type for AUR");
        }
    }

    if (text.isEmpty()) {
        if (!m_filtered)
            return;

        // Show all repo packages
        for (int i = 0; i < model()->packages().count(); ++i)
            setRowHidden(i, QModelIndex(), false);
        m_filtered = false;
        return;
    }

    // Filter local packages
    switch (type) {
    case Name:
        filterPackages(text, &Package::name);
        break;
    case NameDescription:
        filterPackages(text, &Package::name, &Package::description);
        break;
    case Maintainer:
        filterPackages(text, &Package::maintainer);
        break;
    case Description:
        filterPackages(text, &Package::description);
        break;
    }

    m_filtered = true;
}

bool PackagesView::find(const QString &packageName)
{
    clearSelection();

    // Search by name
    for (int i = 0; i < model()->packages().count(); ++i) {
        if (model()->packages().at(i)->name() == packageName) {
            const QModelIndex index = model()->index(i, 0);
            setCurrentIndex(index);
            scrollTo(index);
            return true;
        }
    }

    // If not dound, then search by providing
    for (int i = 0; i < model()->packages().count(); ++i) {
        foreach (const Depend &depend, model()->packages().at(i)->provides()) {
            if (depend.name() == packageName) {
                const QModelIndex index = model()->index(i, 0);
                setCurrentIndex(index);
                scrollTo(index);
                return true;
            }
        }
    }

    return false;
}

Package *PackagesView::currentPackage() const
{
    return static_cast<Package *>(currentIndex().internalPointer());
}

PackagesModel *PackagesView::model() const
{
    return qobject_cast<PackagesModel *>(QTreeView::model());
}

QVector<Package *> PackagesView::installExplicity() const
{
    return m_installExplicity;
}

QVector<Package *> PackagesView::installAsDepend() const
{
    return m_installAsDepend;
}

QVector<Package *> PackagesView::reinstall() const
{
    return m_reinstall;
}

QVector<Package *> PackagesView::markAsExplicit() const
{
    return m_markAsExplicity;
}

QVector<Package *> PackagesView::markAsDepend() const
{
    return m_markAsDepend;
}

QVector<Package *> PackagesView::uninstall() const
{
    return m_uninstall;
}

QVector<Package *> PackagesView::uninstallWithUnused() const
{
    return m_uninstallWithUnused;
}

bool PackagesView::isUpgradePackages() const
{
    return m_upgradePackages;
}

void PackagesView::setUpgradePackages(bool upgrade)
{
    if (m_upgradePackages == upgrade)
        return;

    m_upgradePackages = upgrade;
    emit operationsCountChanged(operationsCount());
}

bool PackagesView::isSyncRepositories() const
{
    return m_syncRepositories;
}

void PackagesView::setSyncRepositories(bool syncRepositories)
{
    if (m_syncRepositories == syncRepositories)
        return;

    m_syncRepositories = syncRepositories;
    emit operationsCountChanged(operationsCount());
}

int PackagesView::operationsCount()
{
    int count = 0;

    count += m_upgradePackages;
    count += m_syncRepositories;
    count += m_installExplicity.size();
    count += m_installAsDepend.size();
    count += m_reinstall.size();
    count += m_markAsExplicity.size();
    count += m_markAsDepend.size();
    count += m_uninstall.size();
    count += m_uninstallWithUnused.size();

    return count;
}

void PackagesView::removeOperation(Task *task)
{
    switch (task->type()) {
    case Task::Sync:
        m_syncRepositories = false;
        break;
    case Task::UpgradeAll:
        m_upgradePackages = false;
        break;
    case Task::Item:
        switch (task->parent()->type()) {
        case Task::InstallExplicity:
            m_installExplicity.removeOne(task->package());
            break;
        case Task::InstallAsDepend:
            m_installAsDepend.removeOne(task->package());
            break;
        case Task::Reinstall:
            m_reinstall.removeOne(task->package());
            break;
        case Task::MarkAsExplicity:
            m_markAsExplicity.removeOne(task->package());
            break;
        case Task::MarkAsDepend:
            m_markAsExplicity.removeOne(task->package());
            break;
        case Task::Uninstall:
            m_uninstall.removeOne(task->package());
            break;
        case Task::UninstallWithUnused:
            m_uninstallWithUnused.removeOne(task->package());
            break;
        default:
            break;
        }
        break;
    case Task::InstallExplicity:
        m_installExplicity.clear();
        break;
    case Task::InstallAsDepend:
        m_installAsDepend.clear();
        break;
    case Task::Reinstall:
        m_reinstall.clear();
        break;
    case Task::MarkAsExplicity:
        m_markAsExplicity.clear();
        break;
    case Task::MarkAsDepend:
        m_markAsExplicity.clear();
        break;
    case Task::Uninstall:
        m_uninstall.clear();
        break;
    case Task::UninstallWithUnused:
        m_uninstallWithUnused.clear();
        break;
    }

    emit operationsCountChanged(operationsCount());
}

void PackagesView::processSelectionChanging(const QModelIndex &current)
{
    auto *package = static_cast<Package *>(current.internalPointer());

    // Load additional AUR info
    if (model()->mode() == PackagesModel::AUR)
        model()->loadMoreAurInfo(package);

    emit currentPackageChanged(package);
}

void PackagesView::processMenuAction(QAction *action)
{
    if (action == m_syncAction)
        setSyncRepositories(true);
    else if (action == m_upgradeAllAction)
        setUpgradePackages(true);
    else if (action == m_installExplicityAction)
        addCurrentToTasks(m_installExplicity);
    else if (action == m_installAsDependAction)
        addCurrentToTasks(m_installAsDepend);
    else if (action == m_reinstallAction)
        addCurrentToTasks(m_reinstall);
    else if (action == m_markAsExplicityAction)
        addCurrentToTasks(m_markAsExplicity);
    else if (action == m_markAsDependAction)
        addCurrentToTasks(m_markAsDepend);
    else if (action == m_uninstallAction)
        addCurrentToTasks(m_uninstall);
    else if (action == m_uninstallWithUnusedAction)
        addCurrentToTasks(m_uninstallWithUnused);
}

void PackagesView::clearAllOperations()
{
    m_upgradePackages = false;
    m_syncRepositories = false;

    m_installExplicity.clear();
    m_installAsDepend.clear();
    m_reinstall.clear();
    m_markAsExplicity.clear();
    m_markAsDepend.clear();
    m_uninstall.clear();
    m_uninstallWithUnused.clear();

    emit operationsCountChanged(0);
}

void PackagesView::contextMenuEvent(QContextMenuEvent *event)
{
    auto *package = static_cast<Package *>(indexAt(event->pos()).internalPointer());
    if (package == nullptr)
        return;

    // Setup menu actions
    if (package->isInstalled()) {
        m_reinstallAction->setVisible(true);
        m_uninstallAction->setVisible(true);
        m_uninstallWithUnusedAction->setVisible(true);

        m_markAsDependAction->setVisible(package->isInstalledExplicitly());
        m_markAsExplicityAction->setVisible(!m_markAsDependAction->isVisible());

        m_installExplicityAction->setVisible(false);
        m_installAsDependAction->setVisible(false);
    } else {
        m_installExplicityAction->setVisible(true);
        m_installAsDependAction->setVisible(true);

        m_reinstallAction->setVisible(false);
        m_markAsDependAction->setVisible(false);
        m_markAsExplicityAction->setVisible(false);
        m_uninstallAction->setVisible(false);
        m_uninstallWithUnusedAction->setVisible(false);
    }

    // Check if opeartions are already selected
    m_installExplicityAction->setEnabled(!m_installExplicity.contains(package));
    m_installAsDependAction->setEnabled(!m_installAsDepend.contains(package));
    m_reinstallAction->setEnabled(!m_reinstall.contains(package));
    m_markAsExplicityAction->setEnabled(!m_markAsExplicity.contains(package));
    m_markAsDependAction->setEnabled(!m_markAsDepend.contains(package));
    m_uninstallAction->setEnabled(!m_uninstall.contains(package));
    m_uninstallWithUnusedAction->setEnabled(!m_uninstallWithUnused.contains(package));
    m_syncAction->setEnabled(!m_syncRepositories);

    // Enable the upgrade option only if upgrades are available or if the sync action is selected
    if ((!model()->outdatedPackages().isEmpty() || m_syncRepositories) && !m_upgradePackages)
        m_upgradeAllAction->setEnabled(true);
    else
        m_upgradeAllAction->setEnabled(false);

    // Disable install operations for AUR for pacman
    const AppSettings settings;
    if (!package->isInstalled() && package->repo() == "aur" && settings.pacmanTool() == AppSettings::defaultPacmanTool()) {
        m_installExplicityAction->setEnabled(false);
        m_installAsDependAction->setEnabled(false);
    } else {
        m_installExplicityAction->setEnabled(true);
        m_installAsDependAction->setEnabled(true);
    }

    m_menu->exec(event->globalPos());
}

void PackagesView::setModel(QAbstractItemModel *model)
{
    QTreeView::setModel(model);
}

void PackagesView::addCurrentToTasks(QVector<Package *> &category)
{
    Package *package = currentPackage();
    removeFromTasks(package);
    category.append(package);

    emit operationsCountChanged(operationsCount());
}

void PackagesView::removeFromTasks(Package *package)
{
    if (m_installExplicity.removeOne(package))
        return;

    if (m_installAsDepend.removeOne(package))
        return;

    if (m_reinstall.removeOne(package))
        return;

    if (m_markAsExplicity.removeOne(package))
        return;

    if (m_markAsDepend.removeOne(package))
        return;

    m_uninstall.removeOne(package);
}

template<class... T>
void PackagesView::filterPackages(const QString &text, T... packagesMembers)
{
    const QStringList splittedText = text.split(' ', QString::SkipEmptyParts);
    for (int i = 0; i < model()->packages().size(); ++i) {
        const Package *package = model()->packages().at(i);

        // Use narrow search
        bool found = true;
        foreach (const QString &textPart, splittedText) {
            if ((!(package->*packagesMembers)().contains(textPart) && ...)) {
                found = false;
                break;
            }
        }
        setRowHidden(i, QModelIndex(), !found);
    }
}
