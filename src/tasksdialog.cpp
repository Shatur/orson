#include "tasksdialog.h"
#include "ui_tasksdialog.h"
#include "pacman.h"
#include "tasks-view/tasksmodel.h"
#include "packages-view/packagesview.h"

#include <QPushButton>
#include <QMenuBar>

TasksDialog::TasksDialog(Pacman *terminal, PackagesView *view, QMenuBar *bar, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TasksDialog),
    m_pacman(terminal),
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

    // Load tasks options from "Tools" menu
    const QMenu *toolsMenu = bar->actions().at(1)->menu();

    // No confirm checkbox
    const QAction *noConfirmAction = toolsMenu->actions().at(0);
    ui->noConfirmCheckBox->setChecked(noConfirmAction->isChecked());
    ui->noConfirmCheckBox->setText(noConfirmAction->iconText());

    // Force checkbox
    const QAction *forceAction = toolsMenu->actions().at(1);
    ui->forceCheckBox->setChecked(forceAction->isChecked());
    ui->forceCheckBox->setText(forceAction->iconText());

    // After completion actions
    const QMenu *afterCompletionMenu = toolsMenu->actions().at(2)->menu();
    ui->afterCompletionlabel->setText(afterCompletionMenu->menuAction()->iconText());
    foreach (QAction *action, afterCompletionMenu->actions())
        ui->afterCompletionComboBox->addItem(action->iconText());
    const int currentIndex = afterCompletionMenu->actions().indexOf(afterCompletionMenu->actions().at(0)->actionGroup()->checkedAction());
    ui->afterCompletionComboBox->setCurrentIndex(currentIndex);
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

void TasksDialog::on_noConfirmCheckBox_toggled(bool checked)
{
    m_pacman->setNoConfirm(checked);
    updateCommandsText();
}

void TasksDialog::on_forceCheckBox_toggled(bool checked)
{
    m_pacman->setForce(checked);
    updateCommandsText();
}

void TasksDialog::on_afterCompletionComboBox_currentIndexChanged(int index)
{
    const auto afterCompletion = static_cast<Pacman::AfterCompletion>(index);
    m_pacman->setAfterTasksCompletion(afterCompletion);
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
    ui->commandsEdit->setPlainText(m_pacman->tasksCommands());
}
