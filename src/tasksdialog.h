#ifndef TASKSDIALOG_H
#define TASKSDIALOG_H

#include <QDialog>

class QMenuBar;
class Pacman;
class PackagesView;

namespace Ui {
class TasksDialog;
}

class TasksDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(TasksDialog)

public:
    TasksDialog(Pacman *terminal, PackagesView *view, QMenuBar *bar, QWidget *parent = nullptr);
    ~TasksDialog() override;

    int exec() override;

private slots:
    void setNoConfirm(bool enabled);
    void setForce(bool enabled);
    void setAfterCompletion(int action);
    void processTaskRemoving();

private:
    void updateCommandsText();

    Ui::TasksDialog *ui;
    Pacman *m_pacman;
    PackagesView *m_packagesView;
};

#endif // TASKSDIALOG_H
