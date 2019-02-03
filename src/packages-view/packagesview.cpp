#include "packagesview.h"
#include "packagesmodel.h"

#include <QHeaderView>
#include <QContextMenuEvent>
#include <QMenu>

PackagesView::PackagesView(QWidget *parent) :
    QTreeView(parent)
{
    // Setup menu
    m_menu = new QMenu(this);
    m_menu->addAction(Task::categoryIcon(Task::Sync), Task::categoryName(Task::Sync));
    m_menu->addAction(Task::categoryIcon(Task::UpgradeAll), Task::categoryName(Task::UpgradeAll));
    m_menu->addAction(Task::categoryIcon(Task::InstallExplicity), Task::categoryName(Task::InstallExplicity));
    m_menu->addAction(Task::categoryIcon(Task::InstallAsDepend), Task::categoryName(Task::InstallAsDepend));
    m_menu->addAction(Task::categoryIcon(Task::Reinstall), Task::categoryName(Task::Reinstall));
    m_menu->addAction(Task::categoryIcon(Task::MarkAsExplicity), Task::categoryName(Task::MarkAsExplicity));
    m_menu->addAction(Task::categoryIcon(Task::MarkAsDepend), Task::categoryName(Task::MarkAsDepend));
    m_menu->addAction(Task::categoryIcon(Task::Uninstall), Task::categoryName(Task::Uninstall));
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

void PackagesView::filter(const QString &text, PackagesView::FilterType type)
{
    // Search packages in AUR
    if (model()->mode() == PackagesModel::AUR) {
        // Detect type
        QString queryType;
        switch (type) {
        case NameDescription:
            queryType = QStringLiteral("name-desc");
            break;
        case Name:
            queryType = QStringLiteral("name");
            break;
        case Maintainer:
            queryType = QStringLiteral("maintainer");
            break;
        default:
            qFatal("Unsupported search type for AUR");
        }

        model()->aurSearch(text, queryType);
        return;
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

    // Filter packages
    switch (type) {
    case Name:
        // Search by name and description
        for (int i = 0; i < model()->packages().size(); ++i) {
            const Package *package = model()->packages().at(i);
            if (package->name().contains(text))
                setRowHidden(i, QModelIndex(), false);
            else
                setRowHidden(i, QModelIndex(), true);
        }
        break;
    case NameDescription:
        // Search only by name
        for (int i = 0; i < model()->packages().size(); ++i) {
            const Package *package = model()->packages().at(i);
            if (package->name().contains(text) || package->description().contains(text))
                setRowHidden(i, QModelIndex(), false);
            else
                setRowHidden(i, QModelIndex(), true);
        }
        break;
    case Maintainer:
        // Search only by name
        for (int i = 0; i < model()->packages().size(); ++i) {
            const Package *package = model()->packages().at(i);
            if (package->maintainer().contains(text))
                setRowHidden(i, QModelIndex(), false);
            else
                setRowHidden(i, QModelIndex(), true);
        }
        break;
    case Description:
        // Search only by description
        for (int i = 0; i < model()->packages().size(); ++i) {
            const Package *package = model()->packages().at(i);
            if (package->description().contains(text))
                setRowHidden(i, QModelIndex(), false);
            else
                setRowHidden(i, QModelIndex(), true);
        }
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

QVector<Package *> PackagesView::uninstall() const
{
    return m_uninstall;
}

QVector<Package *> PackagesView::markAsDepend() const
{
    return m_markAsDepend;
}

QVector<Package *> PackagesView::markAsExplicity() const
{
    return m_markAsExplicity;
}

QVector<Package *> PackagesView::reinstall() const
{
    return m_reinstall;
}

QVector<Package *> PackagesView::installAsDepend() const
{
    return m_installAsDepend;
}

QVector<Package *> PackagesView::installExplicity() const
{
    return m_installExplicity;
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
    emit operationsCountChanged();
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
    emit operationsCountChanged();
}

int PackagesView::operationsCount()
{
    int count = 0;

    if (m_upgradePackages)
        ++count;

    if (m_syncRepositories)
        ++count;

    count += m_installExplicity.size();
    count += m_installAsDepend.size();
    count += m_reinstall.size();
    count += m_markAsExplicity.size();
    count += m_markAsDepend.size();
    count += m_uninstall.size();

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
    }

    emit operationsCountChanged();
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
    const auto category = static_cast<Task::Type>(m_menu->actions().indexOf(action));

    switch (category) {
    case Task::Sync:
        setSyncRepositories(true);
        break;
    case Task::UpgradeAll:
        setUpgradePackages(true);
        break;
    case Task::InstallExplicity:
        addCurrentToTasks(m_installExplicity);
        break;
    case Task::InstallAsDepend:
        addCurrentToTasks(m_installAsDepend);
        break;
    case Task::Reinstall:
        addCurrentToTasks(m_reinstall);
        break;
    case Task::MarkAsExplicity:
        addCurrentToTasks(m_markAsExplicity);
        break;
    case Task::MarkAsDepend:
        addCurrentToTasks(m_markAsDepend);
        break;
    case Task::Uninstall:
        addCurrentToTasks(m_uninstall);
        break;
    default:
        break;
    }
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

    emit operationsCountChanged();
}

void PackagesView::contextMenuEvent(QContextMenuEvent *event)
{
    auto *package = static_cast<Package *>(indexAt(event->pos()).internalPointer());
    if (package == nullptr)
        return;

    // Setup menu actions
    if (package->isInstalled()) {
        m_menu->actions().at(Task::InstallExplicity)->setVisible(false);
        m_menu->actions().at(Task::InstallAsDepend)->setVisible(false);

        m_menu->actions().at(Task::Reinstall)->setVisible(true);
        m_menu->actions().at(Task::Uninstall)->setVisible(true);
        if (package->isInstalledExplicitly())
            m_menu->actions().at(Task::MarkAsDepend)->setVisible(true);
        else
            m_menu->actions().at(Task::MarkAsExplicity)->setVisible(true);
    } else {
        m_menu->actions().at(Task::Reinstall)->setVisible(false);
        m_menu->actions().at(Task::MarkAsDepend)->setVisible(false);
        m_menu->actions().at(Task::MarkAsExplicity)->setVisible(false);
        m_menu->actions().at(Task::Uninstall)->setVisible(false);

        m_menu->actions().at(Task::InstallExplicity)->setVisible(true);
        m_menu->actions().at(Task::InstallAsDepend)->setVisible(true);
    }

    // Check if opeartions are already selected
    m_menu->actions().at(Task::InstallExplicity)->setEnabled(!m_installExplicity.contains(package));
    m_menu->actions().at(Task::InstallAsDepend)->setEnabled(!m_installAsDepend.contains(package));
    m_menu->actions().at(Task::Reinstall)->setEnabled(!m_reinstall.contains(package));
    m_menu->actions().at(Task::MarkAsExplicity)->setEnabled(!m_markAsExplicity.contains(package));
    m_menu->actions().at(Task::MarkAsDepend)->setEnabled(!m_markAsDepend.contains(package));
    m_menu->actions().at(Task::Uninstall)->setEnabled(!m_uninstall.contains(package));
    m_menu->actions().at(Task::Sync)->setEnabled(!m_syncRepositories);

    if ((!model()->outdatedPackages().isEmpty() || m_syncRepositories) && !m_upgradePackages)
        m_menu->actions().at(Task::UpgradeAll)->setEnabled(true);
    else
        m_menu->actions().at(Task::UpgradeAll)->setEnabled(false);

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

    emit operationsCountChanged();
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
