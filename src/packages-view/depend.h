#ifndef DEPEND_H
#define DEPEND_H

#include <alpm.h>

#include <QStringList>

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
    void parseDepend(const QString &text, const QString &mod, int position);

    alpm_depend_t *m_alpmData = nullptr;
    QString m_name;
    QString m_mod;
    QString m_version;
};

#endif // DEPEND_H
