#include "packagesview.h"

#include <QHeaderView>

PackagesView::PackagesView(QWidget *parent) :
    QTreeView(parent)
{
    // Setup items
    setModel(m_model);
    sortByColumn(-1, Qt::AscendingOrder); // Show item unsorted by default
    header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // Add current package changed signal
    connect(selectionModel(), &QItemSelectionModel::currentChanged, [&](const QModelIndex &current) {
        auto package = static_cast<Package *>(current.internalPointer());
        emit currentPackageChanged(package);
    });
}

void PackagesView::filter(const QString &text, PackagesView::SearchType type)
{
    if (text.isEmpty()) {
        for (int i = 0; i < m_model->packages().count(); ++i)
            setRowHidden(i, QModelIndex(), false);
        return;
    }

    switch (type) {
    case Name:
        // Search by name and description
        for (int i = 0; i < m_model->packages().size(); ++i) {
            const Package *package = m_model->packages().at(i);
            if (package->name().contains(text))
                setRowHidden(i, QModelIndex(), false);
            else
                setRowHidden(i, QModelIndex(), true);
        }
        break;
    case NameDescription:
        // Search only by name
        for (int i = 0; i < m_model->packages().size(); ++i) {
            const Package *package = m_model->packages().at(i);
            if (package->name().contains(text) || package->description().contains(text))
                setRowHidden(i, QModelIndex(), false);
            else
                setRowHidden(i, QModelIndex(), true);
        }
        break;
    case Description:
        // Search only by description
        for (int i = 0; i < m_model->packages().size(); ++i) {
            const Package *package = m_model->packages().at(i);
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
    for (int i = 0; i < m_model->packages().count(); ++i) {
        if (m_model->packages().at(i)->name() == packageName) {
            const QModelIndex index = m_model->index(i, 0, QModelIndex());
            setCurrentIndex(index);
            scrollTo(index);
            return;
        }
    }

    // If not dound, then search by providing
    for (int i = 0; i < m_model->packages().count(); ++i) {
        foreach (const alpm_depend_t *dependency, m_model->packages().at(i)->provides()) {
            if (dependency->name == packageName) {
                const QModelIndex index = m_model->index(i, 0, QModelIndex());
                setCurrentIndex(index);
                scrollTo(index);
                return;
            }
        }
    }
}

void PackagesView::selectRow(int row)
{
    const QModelIndex index = m_model->index(row, 0, QModelIndex());
    setCurrentIndex(index);
}

Package *PackagesView::currentPackage() const
{
    return static_cast<Package *>(currentIndex().internalPointer());
}

PackagesModel *PackagesView::model() const
{
    return m_model;
}
