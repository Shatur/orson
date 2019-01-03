#ifndef PACKAGESVIEW_H
#define PACKAGESVIEW_H

#include "packagesmodel.h"

#include <QTreeView>

class PackagesView : public QTreeView
{
    Q_OBJECT

public:
    enum SearchType {
        Name,
        NameDescription,
        Description
    };

    PackagesView(QWidget *parent = nullptr);

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

#endif // PACKAGESVIEW_H
