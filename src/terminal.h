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
    void start();

signals:
    void dataAvailable(const QString &data);
    void finished(int exitCode);

private:
    QPair<QString, QStringList> getTerminalProgram();
    QProcess m_console;
    QString m_commands;
};

#endif // TERMINAL_H
