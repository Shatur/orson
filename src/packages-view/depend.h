#ifndef DEPEND_H
#define DEPEND_H

#include <QStringList>

class _alpm_depend_t;
using alpm_depend_t = _alpm_depend_t;

class Depend
{
public:
    explicit Depend(alpm_depend_t *dependData = nullptr);
    explicit Depend(const QString &text);

    QString name() const;
    QString version() const;
    QString description() const;
    QString mod() const;

private:
    void parseDepend(const QString &text, int modPosition, int modSize = 1);

    alpm_depend_t *m_alpmData = nullptr;
    QString m_name;
    QString m_mod;
    QString m_version;
};

#endif // DEPEND_H
