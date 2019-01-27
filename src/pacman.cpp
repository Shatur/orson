#include "pacman.h"

#include <QFileInfo>
#include <QDebug>

Pacman::Pacman()
{
    m_process.setProcessChannelMode(QProcess::MergedChannels);

    // Add data available signal
    connect(&m_process, &QProcess::readyReadStandardOutput, [&] {
        emit dataAvailable(m_process.readAllStandardOutput());
    });

    // Add finished signal
    connect(&m_process, qOverload<int>(&QProcess::finished), this, &Pacman::finished);
}

void Pacman::setTasks(TasksModel *model)
{
    QStringList pacmanCommands;
    foreach (Task *category, model->categories()) {
        if (category->children().isEmpty())
            continue;

        // Concatenate categories
        if (!pacmanCommands.isEmpty())
            pacmanCommands << "&&";

        // Pacman command
        pacmanCommands << "pacman";
        switch (category->categoryType()) {
        case Task::InstallExplicity:
        case Task::InstallAsDepend:
        case Task::Reinstall:
            pacmanCommands << "-S";
            break;
        case Task::MarkAsDepend:
        case Task::MarkAsExplicity:
            pacmanCommands << "-D";
            break;
        case Task::Uninstall:
            pacmanCommands << "-R";
            break;
        default:
            qFatal("Unknown category");
        }

        // Add packages list
        foreach (Task *task, category->children())
            pacmanCommands << task->name();

        // Reason
        switch (category->categoryType()) {
        case Task::InstallAsDepend:
        case Task::MarkAsDepend:
            pacmanCommands << "--asdeps";
            break;
        case Task::MarkAsExplicity:
            pacmanCommands << "--asexplicit";
            break;
        default:
            break;
        }

        pacmanCommands << "--noconfirm";
    }

    auto [suCommand, suArguments] = getSuCommand();
    m_process.setProgram(suCommand);
    m_process.setArguments(suArguments << pacmanCommands.join(" "));
}

void Pacman::start()
{
    m_process.start();
}

QPair<QString, QStringList> Pacman::getSuCommand()
{
    if (QFileInfo::exists("/usr/bin/pkexec"))
        return {"pkexec", {"script", "-q", "-c"}};
    else if (QFileInfo::exists("/usr/bin/kdesu"))
        return {"kdesu", {"-d", "-t", "--noignorebutton", "-c"}};
    else
        qFatal("No graphical sudo tool found");
}
