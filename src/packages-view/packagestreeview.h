#ifndef PACKAGESTREEVIEW_H
#define PACKAGESTREEVIEW_H

#include "packagesmodel.h"

#include <QTreeView>

class PackagesTreeView : public QTreeView
{
    Q_OBJECT

public:
    PackagesTreeView(QWidget *parent = nullptr);

    void setPackageHidden(Package *package, bool hide);
    void scrollToPackage(Package *package);
    PackagesModel *model() const;

    Package *currentPackage() const;
    void setCurrentPackage(Package *package);

signals:
    void currentPackageChanged(Package *package);

private:
    PackagesModel *m_model = new PackagesModel{this};
};

#endif // PACKAGESTREEVIEW_H
