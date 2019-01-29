#ifndef TERMINAL_H
#define TERMINAL_H

#include "tasks-view/tasksmodel.h"

#include <QProcess>

class Terminal : public QObject
{
    Q_OBJECT

public:
    Terminal();

    void setTasks(TasksModel *model);
    void executeTasks();
    void installPackage(const QString &name, bool asDepend = false);
    void updateDatabase();
    QString commands() const;

    bool isNoConfirm() const;
    void setNoConfirm(bool noconfirm);

signals:
    void dataAvailable(const QString &data);
    void started();
    void finished(int exitCode);

private:
    QPair<QString, QStringList> getTerminalProgram();
    QProcess m_terminal;
    QString m_commands;
    bool m_noConfirm = true;
};

#endif // TERMINAL_H
