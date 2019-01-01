#include "packagestreeview.h"

#include <QHeaderView>

PackagesTreeView::PackagesTreeView(QWidget *parent) :
    QTreeView(parent)
{
    // Setup items
    setModel(m_model);
    header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // Add current package changed signal
    connect(selectionModel(), &QItemSelectionModel::currentChanged, [&](const QModelIndex &current) {
        auto package = static_cast<Package *>(current.internalPointer());
        emit currentPackageChanged(package);
    });
}

void PackagesTreeView::setPackageHidden(Package *package, bool hide)
{
    const int row = m_model->packages().indexOf(package);
    setRowHidden(row, QModelIndex(), hide);
}

void PackagesTreeView::scrollToPackage(Package *package)
{
    const int row = m_model->packages().indexOf(package);
    const QModelIndex index = m_model->index(row, 0, QModelIndex());
    scrollTo(index);
}

PackagesModel *PackagesTreeView::model() const
{
    return m_model;
}

Package *PackagesTreeView::currentPackage() const
{
    return static_cast<Package *>(currentIndex().internalPointer());
}

void PackagesTreeView::setCurrentPackage(Package *package)
{
    const int row = m_model->packages().indexOf(package);
    const QModelIndex index = m_model->index(row, 0, QModelIndex());
    setCurrentIndex(index);
}
