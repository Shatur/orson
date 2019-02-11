#ifndef PACMAN_H
#define PACMAN_H

#include <QObject>
#include <QString>

class QProcess;
class PackagesView;
class Package;

class Pacman : public QObject
{
    Q_OBJECT

public:
    enum AfterCompletion {
        WaitForInput,
        Shutdown,
        Reboot
    };
    Q_ENUM(AfterCompletion)

    Pacman(QObject *parent = nullptr);

    // Actions
    void setTasks(PackagesView *view);
    QString tasksCommands();
    void executeTasks();

    void installPackage(const QString &name, bool asDepend = false);
    void syncDatabase();

    // Parameters
    bool isNoConfirm() const;
    void setNoConfirm(bool noconfirm);

    AfterCompletion afterTasksCompletion() const;
    void setAfterTasksCompletion(const AfterCompletion &afterTasksCompletion);

private slots:
    void getExitCode();

signals:
    void started();
    void finished(int exitCode);

private:
    QPair<QString, QStringList> getTerminalProgram();
    void appendPackagesCommand(QString &tasksCommands, const QString &pacmanTool, const QVector<Package *> &packages, const QString &action, const QString &parameters = QString());
    static QString afterCompletionCommand(AfterCompletion afterCompletion);

    PackagesView *m_tasksView;
    QProcess *m_terminal;
    AfterCompletion m_afterTasksCompletion = WaitForInput;
    bool m_noConfirm = true;
};

#endif // PACMAN_H
