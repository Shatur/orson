#ifndef PACKAGESVIEW_H
#define PACKAGESVIEW_H

#include "packagesmodel.h"
#include "../tasks-view/tasksview.h"

#include <QTreeView>

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
    void setTaskView(TasksView *taskView);
    bool find(const QString &packageName);
    Package *currentPackage() const;
    PackagesModel *model() const;

signals:
    void currentPackageChanged(Package *package);

private slots:
    void addTask(QAction *action);

private:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void setModel(QAbstractItemModel *model) override;

    // Context menu
    QMenu *m_menu;
    QAction *m_installExplicity;
    QAction *m_installAsDepend;
    QAction *m_reinstall;
    QAction *m_uninstall;
    QAction *m_changeReason;

    TasksView *m_taskView;
    bool m_filtered = false;
};

#endif // PACKAGESVIEW_H
