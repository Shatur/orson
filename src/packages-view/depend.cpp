#include "depend.h"

#include <alpm.h>

Depend::Depend(alpm_depend_t *dependData) :
    m_alpmData(dependData)
{
}

Depend::Depend(const QString &text)
{
    int index = text.indexOf('>');
    if (index != -1) {
        parseDepend(text, index);
        return;
    }

    index = text.indexOf('<');
    if (index != -1) {
        parseDepend(text, index);
        return;
    }

    index = text.indexOf('=');
    if (index != -1) {
        parseDepend(text, index);
        return;
    }

    index = text.indexOf(QStringLiteral(">="));
    if (index != -1) {
        parseDepend(text, index, 2);
        return;
    }

    index = text.indexOf(QStringLiteral("<="));
    if (index != -1) {
        parseDepend(text, index, 2);
        return;
    }

    m_name = text;
}

QString Depend::name() const
{
    if (m_alpmData == nullptr)
        return m_name;

    return m_alpmData->name;
}

QString Depend::version() const
{
    if (m_alpmData == nullptr)
        return m_version;

    return m_alpmData->version;
}

QString Depend::description() const
{
    if (m_alpmData == nullptr)
        return QString();

    return m_alpmData->desc;
}

QString Depend::mod() const
{
    if (m_alpmData == nullptr)
        return m_mod;

    switch (m_alpmData->mod) {
    case ALPM_DEP_MOD_EQ:
        return QStringLiteral(" = ");
    case ALPM_DEP_MOD_GE:
        return QStringLiteral(" >= ");
    case ALPM_DEP_MOD_LE:
        return QStringLiteral(" <= ");
    case ALPM_DEP_MOD_GT:
        return QStringLiteral(" > ");
    case ALPM_DEP_MOD_LT:
        return QStringLiteral(" < ");
    default:
        return QString();
    }
}

// Tokenize string like jre>=11 to jre, >= and 11
void Depend::parseDepend(const QString &text, int modPosition, int modSize)
{
    m_name = text.left(modPosition);
    m_mod = text.mid(modPosition, modPosition + modSize);
    m_version = text.right(modPosition + modSize);
}
