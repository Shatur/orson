#include "pacman.h"
#include "packages-view/packagesview.h"
#include "packages-view/package.h"

#include <QFileInfo>
#include <QDebug>
#include <QProcess>

constexpr char pacmanProgram[] = "pikaur ";
const QString errorFile = QStringLiteral("/tmp/orson.err");

Pacman::Pacman(QObject *parent) :
    QObject(parent)
{
    m_terminal = new QProcess(this);
    m_terminal->setProcessChannelMode(QProcess::MergedChannels);
    m_terminal->setProgram("konsole");

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
    QString commands;

    if (m_tasksView->isSyncRepositories() && m_tasksView->isUpgradePackages()) {
        commands.append(pacmanProgram);
        commands.append("-Syu ");
    } else {
        if (m_tasksView->isSyncRepositories()) {
            commands.append(pacmanProgram);
            commands.append("-Sy ");
        }

        if (m_tasksView->isUpgradePackages()) {
            commands.append(pacmanProgram);
            commands.append("-Su ");
        }
    }

    appendPackagesCommand(commands, m_tasksView->installExplicity(), "-S ");
    appendPackagesCommand(commands, m_tasksView->installAsDepend(), "-S ", "--asdepend ");
    appendPackagesCommand(commands, m_tasksView->reinstall(), "-S ");
    appendPackagesCommand(commands, m_tasksView->markAsExplicity(), "-D ", "--asexplicity ");
    appendPackagesCommand(commands, m_tasksView->markAsDepend(), "-D ", "--asdepend ");
    appendPackagesCommand(commands, m_tasksView->uninstall(), "-R ");

    commands.chop(1); // Remove last space character
    return commands;
}

void Pacman::executeTasks()
{
    auto [terminal, terminalArguments] = getTerminalProgram();
    m_terminal->setProgram(terminal);
    m_terminal->setArguments(terminalArguments << tasksCommands() + afterCompletionCommand(m_afterTasksCompletion));

    m_terminal->start();
}

void Pacman::installPackage(const QString &name, bool asDepend)
{
    QString command = "sudo pacman -U " + name;
    if (asDepend)
        command += " --asdepend";

    auto [terminal, terminalArguments] = getTerminalProgram();
    m_terminal->setProgram(terminal);
    m_terminal->setArguments(terminalArguments << command + afterCompletionCommand(WaitForInput));
    m_terminal->start();
}

void Pacman::syncDatabase()
{
    const QString command = QStringLiteral("sudo pacman -Sy");

    auto [terminal, terminalArguments] = getTerminalProgram();
    m_terminal->setProgram(terminal);
    m_terminal->setArguments(terminalArguments << command + afterCompletionCommand(WaitForInput));
    m_terminal->start();
}

QPair<QString, QStringList> Pacman::getTerminalProgram()
{
    QPair<QString, QStringList> program;
    if (QFileInfo::exists("/usr/bin/konsole"))
        program = {"konsole", {"--hide-menubar", "--hide-tabbar", "-e"}};
    else if (QFileInfo::exists("/usr/bin/gnome-terminal"))
        program = {"gnome-terminal", {"--hide-menubar", "--"}};
    else
        qFatal("Unable to found terminal program");

    // Add shell execution
    program.second << "$SHELL" << "-c";
    return program;
}

void Pacman::appendPackagesCommand(QString &commands, const QVector<Package *> &packages, const QString &action, const QString &parameters)
{
    if (packages.isEmpty())
        return;

    if (!commands.isEmpty())
        commands.append("&& ");

    commands.append(pacmanProgram);
    commands.append(action);
    commands.append(parameters);

    foreach (Package *package, packages)
        commands.append(package->name() + " ");

    if (m_noConfirm)
        commands.append("--noconfirm ");
}

QString Pacman::afterCompletionCommand(AfterCompletion afterCompletion)
{
    QString command;
    switch (afterCompletion) {
    case Shutdown:
        command.append(" && sudo shutdown 0");
        break;
    case Reboot:
        command.append(" && sudo reboot");
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

    if (!errorFile.exists()) {
        emit finished(0);
        return;
    }

    errorFile.open(QIODevice::ReadWrite);
    emit finished(errorFile.readAll().toInt());
    errorFile.remove();
}

bool Pacman::isNoConfirm() const
{
    return m_noConfirm;
}

void Pacman::setNoConfirm(bool noconfirm)
{
    m_noConfirm = noconfirm;
}