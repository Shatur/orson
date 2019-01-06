#ifndef PACKAGESVIEW_H
#define PACKAGESVIEW_H

#include "packagesmodel.h"
#include <QTreeView>

class PackagesView : public QTreeView
{
    Q_OBJECT

public:
    enum FilterType {
        NameDescription,
        Name,
        Description
    };

    explicit PackagesView(QWidget *parent = nullptr);

    void filter(const QString &text, FilterType type = NameDescription);
    void find(const QString &packageName);

    Package *currentPackage() const;
    PackagesModel *model() const;

signals:
    void currentPackageChanged(Package *package);
};

#endif // PACKAGESVIEW_H
