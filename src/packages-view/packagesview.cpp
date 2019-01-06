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
        auto package = static_cast<Package *>(current.internalPointer());
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
            queryType = "name-desc";
            break;
        case Name:
            queryType = "name";
            break;
        default:
            qFatal("Unsupported search type for AUR");
        }

        model()->aurSearch(text, queryType);
        return;
    }

    if (text.isEmpty()) {
        // Show all repo packages
        for (int i = 0; i < model()->packages().count(); ++i)
            setRowHidden(i, QModelIndex(), false);
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
}

void PackagesView::find(const QString &packageName)
{
    clearSelection();

    // Search by name
    for (int i = 0; i < model()->packages().count(); ++i) {
        if (model()->packages().at(i)->name() == packageName) {
            const QModelIndex index = model()->index(i, 0);
            setCurrentIndex(index);
            scrollTo(index);
            return;
        }
    }

    // If not dound, then search by providing
    for (int i = 0; i < model()->packages().count(); ++i) {
        foreach (const alpm_depend_t *depend, model()->packages().at(i)->provides()) {
            if (depend->name == packageName) {
                const QModelIndex index = model()->index(i, 0);
                setCurrentIndex(index);
                scrollTo(index);
                return;
            }
        }
    }
}

Package *PackagesView::currentPackage() const
{
    return static_cast<Package *>(currentIndex().internalPointer());
}

PackagesModel *PackagesView::model() const
{
    return static_cast<PackagesModel *>(QTreeView::model());
}
