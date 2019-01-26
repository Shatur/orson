#ifndef PACMAN_H
#define PACMAN_H

#include "tasks-view/tasksmodel.h"

#include <QProcess>

class Pacman : public QObject
{
    Q_OBJECT

public:
    Pacman();

    void setTasks(TasksModel *model);
    void start();

signals:
    void dataAvailable(const QString &data);

private:
    static QPair<QString, QStringList> getSuCommand();

    QProcess m_process;
};

#endif // PACMAN_H
