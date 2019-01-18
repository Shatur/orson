#include "packagesview.h"
#include "packagesmodel.h"

#include <QHeaderView>

PackagesView::PackagesView(QWidget *parent) :
    QTreeView(parent)
{
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

void PackagesView::setModel(QAbstractItemModel *model)
{
    QTreeView::setModel(model);
}
