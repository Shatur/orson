#include "terminal.h"

#include <QFileInfo>
#include <QDebug>

Terminal::Terminal()
{
    m_console.setProcessChannelMode(QProcess::MergedChannels);
    m_console.setProgram("konsole");

    // Add data available signal
    connect(&m_console, &QProcess::readyReadStandardOutput, [&] {
        emit dataAvailable(m_console.readAllStandardOutput());
    });

    // Add finished signal
    connect(&m_console, qOverload<int>(&QProcess::finished), this, &Terminal::finished);
}

void Terminal::setTasks(TasksModel *model)
{
    m_commands.clear();
    constexpr char pacmanProgram[] = "pikaur ";

    foreach (Task *category, model->categories()) {
        if (category->children().isEmpty())
            continue;

        // Concatenate categories
        if (!m_commands.isEmpty())
            m_commands.append(" && ");

        // Pacman command
        m_commands.append(pacmanProgram);
        switch (category->categoryType()) {
        case Task::InstallExplicity:
        case Task::InstallAsDepend:
        case Task::Reinstall:
            m_commands.append("-S ");
            break;
        case Task::MarkAsDepend:
        case Task::MarkAsExplicity:
            m_commands.append("-D ");
            break;
        case Task::Uninstall:
            m_commands.append("-R ");
            break;
        default:
            qFatal("Unknown category");
        }

        // Add packages list
        foreach (Task *task, category->children())
            m_commands.append(task->name() + " ");

        // Reason
        switch (category->categoryType()) {
        case Task::InstallAsDepend:
        case Task::MarkAsDepend:
            m_commands.append("--asdeps ");
            break;
        case Task::MarkAsExplicity:
            m_commands.append("--asexplicit ");
            break;
        default:
            break;
        }

        if (m_noConfirm)
            m_commands.append("--noconfirm");
    }
}

void Terminal::executeTasks()
{
    constexpr char waitForInput[] = " && read -r -s -n 1";

    auto [terminal, terminalArguments] = getTerminalProgram();
    m_console.setProgram(terminal);
    m_console.setArguments(terminalArguments << m_commands + waitForInput);
    m_console.start();
}

QPair<QString, QStringList> Terminal::getTerminalProgram()
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

bool Terminal::isNoConfirm() const
{
    return m_noConfirm;
}

void Terminal::setNoConfirm(bool noconfirm)
{
    m_noConfirm = noconfirm;
}

QString Terminal::commands() const
{
    return m_commands;
}
