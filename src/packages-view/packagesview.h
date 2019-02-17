#ifndef PACKAGESVIEW_H
#define PACKAGESVIEW_H

#include <QTreeView>

class Task;
class Package;
class PackagesModel;

class PackagesView : public QTreeView
{
    Q_OBJECT

public:
    enum FilterType {
        NameDescription,
        Name,
        Maintainer,
        Description
    };

    explicit PackagesView(QWidget *parent = nullptr);

    void filter(const QString &text, FilterType type = NameDescription);
    bool find(const QString &packageName);
    Package *currentPackage() const;
    PackagesModel *model() const;

    // Packages operations
    QVector<Package *> installExplicity() const;
    QVector<Package *> installAsDepend() const;
    QVector<Package *> reinstall() const;
    QVector<Package *> markAsExplicit() const;
    QVector<Package *> markAsDepend() const;
    QVector<Package *> uninstall() const;
    QVector<Package *> uninstallWithUnused() const;

    bool isUpgradePackages() const;
    void setUpgradePackages(bool isUpgradePackages);

    bool isSyncRepositories() const;
    void setSyncRepositories(bool isSyncRepositories);

    int operationsCount();

public slots:
    void removeOperation(Task *task);

signals:
    void currentPackageChanged(Package *package);
    void operationsCountChanged(int count);

private slots:
    void processSelectionChanging(const QModelIndex &current);
    void processMenuAction(QAction *action);
    void clearAllOperations();

private:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void setModel(QAbstractItemModel *model) override;

    void addCurrentToTasks(QVector<Package *> &category);
    void removeFromTasks(Package *package);

    // Context menu actions
    QAction *m_installExplicityAction;
    QAction *m_installAsDependAction;
    QAction *m_reinstallAction;
    QAction *m_markAsExplicityAction;
    QAction *m_markAsDependAction;
    QAction *m_uninstallAction;
    QAction *m_syncAction;
    QAction *m_upgradeAllAction;
    QAction *m_uninstallWithUnusedAction;

    // Packages operations
    QVector<Package *> m_installExplicity;
    QVector<Package *> m_installAsDepend;
    QVector<Package *> m_reinstall;
    QVector<Package *> m_markAsExplicity;
    QVector<Package *> m_markAsDepend;
    QVector<Package *> m_uninstall;
    QVector<Package *> m_uninstallWithUnused;
    bool m_upgradePackages = false;
    bool m_syncRepositories = false;

    bool m_filtered = false;
    QMenu *m_menu;
};

#endif // PACKAGESVIEW_H
