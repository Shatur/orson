#ifndef TASKSDIALOG_H
#define TASKSDIALOG_H

#include "tasks-view/tasksview.h"
#include "terminal.h"

#include <QDialog>

namespace Ui {
class TasksDialog;
}

class TasksDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TasksDialog(Terminal *terminal, QWidget *parent = nullptr);
    ~TasksDialog() override;

    virtual int exec() override;
    TasksView *view();

private slots:
    void processTaskRemoving();

private:
    void updateCommandsText();

    Ui::TasksDialog *ui;
    Terminal *m_terminal;
};

#endif // TASKSDIALOG_H
