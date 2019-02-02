#include "tasksdialog.h"
#include "ui_tasksdialog.h"

#include <QPushButton>

TasksDialog::TasksDialog(Pacman *terminal, PackagesView *view, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TasksDialog),
    m_terminal(terminal),
    m_packagesView(view)
{
    ui->setupUi(this);
    ui->tasksView->model()->setTasks(m_packagesView);
    ui->tasksView->expandAll();
    connect(m_packagesView, &PackagesView::operationsCountChanged, this, &TasksDialog::processTaskRemoving);

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
    if (m_packagesView->operationsCount() > 0)
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    else
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    updateCommandsText();
}

void TasksDialog::updateCommandsText()
{
    m_terminal->setTasks(m_packagesView);
    ui->commandsEdit->setPlainText(m_terminal->commands());
}
