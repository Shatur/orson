#ifndef TASKSDIALOG_H
#define TASKSDIALOG_H

#include "tasks-view/tasksview.h"
#include "pacman.h"

#include <QDialog>

namespace Ui {
class TasksDialog;
}

class TasksDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TasksDialog(Pacman *terminal, PackagesView *view, QWidget *parent = nullptr);
    ~TasksDialog() override;

    virtual int exec() override;
    TasksView *view();

private slots:
    void processTaskRemoving();

private:
    void updateCommandsText();

    Ui::TasksDialog *ui;
    Pacman *m_terminal;
    PackagesView *m_packagesView;
};

#endif // TASKSDIALOG_H
