#include "pacman.h"
#include "appsettings.h"
#include "packages-view/packagesview.h"
#include "packages-view/package.h"

#include <QFileInfo>
#include <QDebug>
#include <QProcess>
#include <QMessageBox>

const QString errorFile = QStringLiteral("/tmp/orson.err");

Pacman::Pacman(QObject *parent) :
    QObject(parent)
{
    m_terminal = new QProcess(this);

    // Add finished and started signal
    connect(m_terminal, qOverload<int>(&QProcess::finished), this, &Pacman::getExitCode);
    connect(m_terminal, &QProcess::started, this, &Pacman::started);
}

void Pacman::setTasks(PackagesView *view)
{
    m_tasksView = view;
}

QString Pacman::tasksCommands()
{
    const AppSettings settings;
    const QString pacmanTool = settings.pacmanTool();
    QString commands;

    if (m_tasksView->isSyncRepositories() && m_tasksView->isUpgradePackages()) {
        commands.append(pacmanTool);
        commands.append(" -Syu");
        if (m_noConfirm)
            commands.append(" --noconfirm");
        if (m_force)
            commands.append(" --force");
    } else {
        if (m_tasksView->isSyncRepositories()) {
            commands.append(pacmanTool);
            commands.append(" -Sy");
        }

        if (m_tasksView->isUpgradePackages()) {
            commands.append(pacmanTool);
            commands.append(" -Su");
            if (m_noConfirm)
                commands.append(" --noconfirm");
            if (m_force)
                commands.append(" --force");
        }
    }

    appendPackagesCommand(commands, pacmanTool, m_tasksView->installExplicity(), " -S");
    appendPackagesCommand(commands, pacmanTool, m_tasksView->installAsDepend(), " -S", " --asdeps");
    appendPackagesCommand(commands, pacmanTool, m_tasksView->reinstall(), " -S");
    appendPackagesCommand(commands, pacmanTool, m_tasksView->markAsExplicit(), " -D", " --asexplicit");
    appendPackagesCommand(commands, pacmanTool, m_tasksView->markAsDepend(), " -D", " --asdeps");
    appendPackagesCommand(commands, pacmanTool, m_tasksView->uninstall(), " -R");
    appendPackagesCommand(commands, pacmanTool, m_tasksView->uninstallWithUnused(), " -Rs");

    return commands;
}

void Pacman::executeTasks()
{
    if (m_tasksView->isSyncRepositories())
        m_updateTimeOnSuccess = true;
    exec(tasksCommands(), m_afterTasksCompletion);
}

void Pacman::installLocalPackage(const QString &fileName, bool asDepend)
{
    QString command = "sudo pacman -U " + fileName;
    if (asDepend)
        command += " --asdepend";

    exec(command, WaitForInput);
}

void Pacman::syncDatabase()
{
    m_terminal->setProgram("systemctl");
    m_terminal->setArguments({ "start", "orson-sync"});
    m_terminal->start();
}

bool Pacman::isNoConfirm() const
{
    return m_noConfirm;
}

void Pacman::setNoConfirm(bool noconfirm)
{
    m_noConfirm = noconfirm;
}

bool Pacman::isForce() const
{
    return m_force;
}

void Pacman::setForce(bool force)
{
    m_force = force;
}

Pacman::AfterCompletion Pacman::afterTasksCompletion() const
{
    return m_afterTasksCompletion;
}

void Pacman::setAfterTasksCompletion(const AfterCompletion &afterTasksCompletion)
{
    m_afterTasksCompletion = afterTasksCompletion;
}

void Pacman::getExitCode()
{
    QFile errorFile("/tmp/orson.err");

    // No error
    if (!errorFile.exists()) {
        if (m_updateTimeOnSuccess) {
            AppSettings settings;
            settings.setLastSync(QDateTime::currentDateTime());
            m_updateTimeOnSuccess = false;
        }
        
        emit finished(0);
        return;
    }

    // Read error
    m_updateTimeOnSuccess = false;
    errorFile.open(QIODevice::ReadWrite);
    emit finished(errorFile.readAll().toInt());
    errorFile.remove();
}

void Pacman::appendPackagesCommand(QString &commands, const QString &pacmanTool, const QVector<Package *> &packages, const QString &action, const QString &parameters)
{
    if (packages.isEmpty())
        return;

    if (!commands.isEmpty())
        commands.append(" && ");

    commands.append(pacmanTool);
    commands.append(action);

    foreach (Package *package, packages)
        commands.append(" " + package->name());

    commands.append(parameters);

    if (m_noConfirm)
        commands.append(" --noconfirm");

    if (m_force)
        commands.append(" --force");
}

QString Pacman::afterCompletionCommand(AfterCompletion afterCompletion)
{
    QString command;
    switch (afterCompletion) {
    case Shutdown:
        command.append(" && shutdown -h now");
        break;
    case Reboot:
        command.append(" && shutdown -r now");
        break;
    default:
        break;
    }

    command.append(" && echo");
    command.append(" && read -s -p '" + tr("Success! To close this window, press <Enter>...") + "'");
    command.append(" || (echo $? > " + errorFile);
    command.append(" && echo");
    command.append(" && read -s -p '" + tr("Failed! To close this window, press <Enter>...") + "')");

    return command;
}

void Pacman::exec(const QString &commands, Pacman::AfterCompletion afterCompletion)
{
    const AppSettings settings;
    m_terminal->setProgram(settings.terminal());
    if (m_terminal->program().isEmpty()) {
        QMessageBox message;
        message.setIcon(QMessageBox::Critical);
        message.setWindowTitle("Terminal emulator not found");
        message.setText("Unable to find terminal emulator program. Please specify it in the settings.");
        message.exec();
        return;
    }

    QStringList terminalArguments = settings.terminalArguments(m_terminal->program());
    terminalArguments << "bash" << "-c"; // Execute shell to launch several commands
    m_terminal->setArguments(terminalArguments << commands + afterCompletionCommand(afterCompletion));
    m_terminal->start();
}
