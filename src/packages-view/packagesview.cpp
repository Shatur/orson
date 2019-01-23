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
    m_menu->addAction(QIcon::fromTheme("filesaveas"), "Install explicity");
    m_menu->addAction(QIcon::fromTheme("filesaveas"), "Install as dependency");
    m_menu->addAction(QIcon::fromTheme("filesaveas"), "Reinstall");
    m_menu->addAction(QIcon::fromTheme("edit"), "Mark as explicity");
    m_menu->addAction(QIcon::fromTheme("edit"), "Mark as dependency");
    m_menu->addAction(QIcon::fromTheme("remove"), "Uninstall");
    connect(m_menu, &QMenu::triggered, this, &PackagesView::addTask);

    // Setup items
    sortByColumn(-1, Qt::AscendingOrder); // Show item unsorted by default
    setModel(new PackagesModel(this));
    header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    connect(selectionModel(), &QItemSelectionModel::currentChanged, this, &PackagesView::changeCurrent);

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

void PackagesView::setTaskView(TasksView *taskView)
{
    m_taskView = taskView;
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

void PackagesView::changeCurrent(const QModelIndex &current)
{
    auto *package = static_cast<Package *>(current.internalPointer());

    // Load additional AUR info
    if (model()->mode() == PackagesModel::AUR)
        model()->loadMoreAurInfo(package);

    emit currentPackageChanged(package);
}

void PackagesView::addTask(QAction *action)
{
    const auto category = static_cast<Task::Category>(m_menu->actions().indexOf(action));
    m_taskView->model()->addTask(currentPackage(), category);
}

void PackagesView::contextMenuEvent(QContextMenuEvent *event)
{
    const auto *package = static_cast<Package *>(indexAt(event->pos()).internalPointer());
    if (package == nullptr)
        return;

    // Disable an action if the selected package has already been added to its category
    Task *task = m_taskView->model()->find(package->name());
    for (int category = 0; category < m_menu->actions().size(); ++category) {
        if (task != nullptr && category == task->parent()->category())
            m_menu->actions().at(category)->setEnabled(false);
        else
            m_menu->actions().at(category)->setEnabled(true);
    }

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

    m_menu->exec(event->globalPos());
}

void PackagesView::setModel(QAbstractItemModel *model)
{
    QTreeView::setModel(model);
}
