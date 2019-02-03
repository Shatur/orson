#include "pacman.h"

#include <QFileInfo>
#include <QDebug>

constexpr char pacmanProgram[] = "pikaur ";
constexpr char waitForInput[] = " && echo && read -s -p 'Successfully! To close this window, press <Enter>...'";
constexpr char shutdown[] = " && sudo shutdown 0";
constexpr char reboot[] = " && sudo reboot";

Pacman::Pacman(QObject *parent) :
    QObject(parent)
{
    m_terminal.setProcessChannelMode(QProcess::MergedChannels);
    m_terminal.setProgram("konsole");

    // Add data available signal
    connect(&m_terminal, &QProcess::readyReadStandardOutput, [&] {
        emit dataAvailable(m_terminal.readAllStandardOutput());
    });

    // Add finished and started signal
    connect(&m_terminal, qOverload<int>(&QProcess::finished), this, &Pacman::finished);
    connect(&m_terminal, &QProcess::started, this, &Pacman::started);
}

void Pacman::setTasks(PackagesView *view)
{
    m_commands.clear();

    if (view->isSyncRepositories() && view->isUpgradePackages()) {
        m_commands.append(pacmanProgram);
        m_commands.append("-Syu ");
    } else {
        if (view->isSyncRepositories()) {
            m_commands.append(pacmanProgram);
            m_commands.append("-Sy ");
        }

        if (view->isUpgradePackages()) {
            m_commands.append(pacmanProgram);
            m_commands.append("-Su ");
        }
    }

    addPackages(view->installExplicity(), "-S ");
    addPackages(view->installAsDepend(), "-S ", "--asdepend ");
    addPackages(view->reinstall(), "-S ");
    addPackages(view->markAsExplicity(), "-D ", "--asexplicity ");
    addPackages(view->markAsDepend(), "-D ", "--asdepend ");
    addPackages(view->uninstall(), "-R ");

    m_commands.chop(1); // Remove last space character
}

void Pacman::executeTasks()
{
    auto [terminal, terminalArguments] = getTerminalProgram();
    m_terminal.setProgram(terminal);
    switch (m_afterTasksCompletion) {
    case WaitForInput:
        m_terminal.setArguments(terminalArguments << m_commands + waitForInput);
        break;
    case Shutdown:
        m_terminal.setArguments(terminalArguments << m_commands + shutdown);
        break;
    case Reboot:
        m_terminal.setArguments(terminalArguments << m_commands + reboot);
        break;
    default:
        m_terminal.setArguments(terminalArguments << m_commands);
        break;
    }

    m_terminal.start();
}

void Pacman::installPackage(const QString &name, bool asDepend)
{
    QString command = "sudo pacman -U " + name;
    if (asDepend)
        command += " --asdepend";

    auto [terminal, terminalArguments] = getTerminalProgram();
    m_terminal.setProgram(terminal);
    m_terminal.setArguments(terminalArguments << command + waitForInput);
    m_terminal.start();
}

void Pacman::syncDatabase()
{
    const QString command = QStringLiteral("sudo pacman -Sy");

    auto [terminal, terminalArguments] = getTerminalProgram();
    m_terminal.setProgram(terminal);
    m_terminal.setArguments(terminalArguments << command + waitForInput);
    m_terminal.start();
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

void Pacman::addPackages(const QVector<Package *> &packages, const QString &command, const QString &arguments)
{
    if (packages.isEmpty())
        return;

    if (!m_commands.isEmpty())
        m_commands.append("&& ");

    m_commands.append(pacmanProgram);
    m_commands.append(command);
    m_commands.append(arguments);

    foreach (Package *package, packages)
        m_commands.append(package->name() + " ");

    if (m_noConfirm)
        m_commands.append("--noconfirm ");
}

Pacman::AfterCompletion Pacman::afterTasksCompletion() const
{
    return m_afterTasksCompletion;
}

void Pacman::setAfterTasksCompletion(const AfterCompletion &afterTasksCompletion)
{
    m_afterTasksCompletion = afterTasksCompletion;
}

bool Pacman::isNoConfirm() const
{
    return m_noConfirm;
}

void Pacman::setNoConfirm(bool noconfirm)
{
    m_noConfirm = noconfirm;
}

QString Pacman::commands() const
{
    return m_commands;
}
