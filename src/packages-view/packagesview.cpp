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
    m_installExplicity = new QAction(QIcon::fromTheme("filesaveas"), "Install", m_menu);
    m_installAsDepend = new QAction(QIcon::fromTheme("filesaveas"), "Install as dependency", m_menu);
    m_reinstall = new QAction(QIcon::fromTheme("filesaveas"), "Reinstall", m_menu);
    m_remove = new QAction(QIcon::fromTheme("remove"), "Remove", m_menu);
    m_changeReason = new QAction(QIcon::fromTheme("edit"), QString(), m_menu);
    m_menu->addAction(m_installExplicity);
    m_menu->addAction(m_installAsDepend);
    m_menu->addAction(m_reinstall);
    m_menu->addAction(m_remove);
    m_menu->addAction(m_changeReason);

    // Setup items
    sortByColumn(-1, Qt::AscendingOrder); // Show item unsorted by default
    setModel(new PackagesModel(this));
    header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // Add current package changed signal
    connect(selectionModel(), &QItemSelectionModel::currentChanged, [&](const QModelIndex &current) {
        auto *package = static_cast<Package *>(current.internalPointer());

        // Load additional AUR info
        if (model()->mode() == PackagesModel::AUR)
            model()->loadMoreAurInfo(package);

        emit currentPackageChanged(package);
    });

    // Emit current package changed signal if package data changed too
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

void PackagesView::contextMenuEvent(QContextMenuEvent *event)
{
    const auto *package = static_cast<Package *>(indexAt(event->pos()).internalPointer());
    if (package == nullptr)
        return;

    // Setup menu items
    if (package->isInstalled()) {
        m_installAsDepend->setVisible(false);
        m_installExplicity->setVisible(false);

        m_remove->setVisible(true);
        m_reinstall->setVisible(true);
        m_changeReason->setVisible(true);
        if (package->isInstalledExplicitly())
            m_changeReason->setText("Mark installed as dependency");
        else
            m_changeReason->setText("Mark installed explicity");
    } else {
        m_reinstall->setVisible(false);
        m_changeReason->setVisible(false);
        m_remove->setVisible(false);

        m_installAsDepend->setVisible(true);
        m_installExplicity->setVisible(true);
    }

    m_menu->exec(event->globalPos());
}

void PackagesView::setModel(QAbstractItemModel *model)
{
    QTreeView::setModel(model);
}
