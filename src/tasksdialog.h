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

public:
    TasksDialog(Pacman *terminal, PackagesView *view, QMenuBar *bar, QWidget *parent = nullptr);
    ~TasksDialog() override;

    virtual int exec() override;

private slots:
    void on_noConfirmCheckBox_toggled(bool checked);
    void on_forceCheckBox_toggled(bool checked);
    void on_afterCompletionComboBox_currentIndexChanged(int index);

    void processTaskRemoving();

private:
    void updateCommandsText();

    Ui::TasksDialog *ui;
    Pacman *m_pacman;
    PackagesView *m_packagesView;
};

#endif // TASKSDIALOG_H
