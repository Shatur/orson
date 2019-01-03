#ifndef PACKAGESTREEVIEW_H
#define PACKAGESTREEVIEW_H

#include "packagesmodel.h"

#include <QTreeView>

class PackagesTreeView : public QTreeView
{
    Q_OBJECT

public:
    enum SearchType {
        Name,
        NameDescription,
        Description
    };

    PackagesTreeView(QWidget *parent = nullptr);

    void filter(const QString &text, SearchType type = Name);
    void find(const QString &packageName);
    void selectRow(int row);
    Package *currentPackage() const;
    PackagesModel *model() const;

signals:
    void currentPackageChanged(Package *package);

private:
    PackagesModel *m_model = new PackagesModel{this};
};

#endif // PACKAGESTREEVIEW_H
