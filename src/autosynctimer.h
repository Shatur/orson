#ifndef AUTOSYNCTIMER_H
#define AUTOSYNCTIMER_H

#include <QObject>

class QTimer;

class AutosyncTimer : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AutosyncTimer)

public:
    enum AutosyncType {
        NoAutosync,
        Interval,
        SpecifiedTime
    };
    Q_ENUM(AutosyncType)

    AutosyncTimer(QObject *parent = nullptr);

    void loadSettings();

signals:
    void timeout();

private slots:
    void processTimeout();

private:
    int intervalToMsec(int hours);
    int timeToMsec(QTime time);

    QTimer *m_timer;
    AutosyncType m_autosyncType = NoAutosync;

};

#endif // AUTOSYNCTIMER_H
