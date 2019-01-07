#ifndef DEPEND_H
#define DEPEND_H

#include <alpm.h>

#include <QString>

class Depend
{
public:
    explicit Depend(alpm_depend_t *dependData = nullptr);

    QString name() const;
    QString version() const;
    QString description() const;
    QString mod() const;

private:
    alpm_depend_t *m_alpmData;
};

#endif // DEPEND_H
