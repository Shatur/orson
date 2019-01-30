#include "tasksdialog.h"
#include "ui_tasksdialog.h"

#include <QPushButton>

TasksDialog::TasksDialog(Terminal *terminal, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TasksDialog),
    m_terminal(terminal)
{
    ui->setupUi(this);
    connect(ui->tasksView->model(), &TasksModel::taskRemoved, this, &TasksDialog::processTaskRemoving);

    // Change OK button text and icon
    QPushButton *okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    okButton->setText("Launch in terminal");
    okButton->setIcon(QIcon::fromTheme("utilities-terminal"));
}

TasksDialog::~TasksDialog()
{
    delete ui;
}

int TasksDialog::exec()
{
    updateCommandsText();
    return QDialog::exec();
}

TasksView *TasksDialog::view()
{
    return ui->tasksView;
}

void TasksDialog::processTaskRemoving()
{
    if (ui->tasksView->model()->allTasksCount() > 0)
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    else
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    updateCommandsText();
}

void TasksDialog::updateCommandsText()
{
    m_terminal->setTasks(ui->tasksView->model());
    ui->commandsEdit->setPlainText(m_terminal->commands());
}
