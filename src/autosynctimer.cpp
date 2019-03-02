#include "autosynctimer.h"
#include "appsettings.h"

#include <QTimer>
#include <QTime>

AutosyncTimer::AutosyncTimer(QObject *parent) :
    QObject(parent)
{
    m_timer = new QTimer(this);
    m_timer->setTimerType(Qt::VeryCoarseTimer); // We don't need millisecond accuracy
    connect(m_timer, &QTimer::timeout, this, &AutosyncTimer::processTimeout);
}

void AutosyncTimer::loadSettings()
{
    const AppSettings settings;
    m_autosyncType = settings.autosyncType();

    switch (m_autosyncType) {
    case NoAutosync:
        m_timer->stop();
        break;
    case Interval:
        m_timer->setInterval(intervalToMsec(settings.autosyncInterval()));
        m_timer->start();
        break;
    case SpecifiedTime:
        m_timer->setInterval(timeToMsec(settings.autosyncTime()));
        m_timer->start();
        break;
    }
}

void AutosyncTimer::processTimeout()
{
    if (m_autosyncType == SpecifiedTime) {
        const AppSettings settings;
        const int msecsToNextSync = timeToMsec(settings.autosyncTime());
        m_timer->setInterval(msecsToNextSync);
    }

    emit timeout();
}

int AutosyncTimer::intervalToMsec(int hours)
{
    constexpr int msecsInHour = 3600000;
    return hours * msecsInHour;
}

int AutosyncTimer::timeToMsec(QTime time)
{
    int msecs = QTime::currentTime().msecsTo(time);
    if (msecs < 2000) {
        constexpr int msecsInDay = 86400000;
        msecs += msecsInDay; // Need to convert from earlier time (https://doc.qt.io/qt-5/qtime.html#msecsTo)
    }

    return msecs;
}
