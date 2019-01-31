#ifndef TERMINAL_H
#define TERMINAL_H

#include "tasks-view/tasksmodel.h"

#include <QProcess>

class Terminal : public QObject
{
    Q_OBJECT

public:
    enum AfterCompletion {
        CloseTerminal,
        WaitForInput,
        Shutdown,
        Reboot
    };

    Terminal(QObject *parent = nullptr);

    void setTasks(TasksModel *model);
    void executeTasks();
    void installPackage(const QString &name, bool asDepend = false);
    void syncDatabase();
    QString commands() const;

    bool isNoConfirm() const;
    void setNoConfirm(bool noconfirm);

    AfterCompletion afterTasksCompletion() const;
    void setAfterTasksCompletion(const AfterCompletion &afterTasksCompletion);

signals:
    void dataAvailable(const QString &data);
    void started();
    void finished(int exitCode);

private:
    QPair<QString, QStringList> getTerminalProgram();
    QProcess m_terminal;
    QString m_commands;
    AfterCompletion m_afterTasksCompletion = WaitForInput;
    bool m_noConfirm = true;
};

#endif // TERMINAL_H
