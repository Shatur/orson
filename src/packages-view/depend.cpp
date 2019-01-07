#include "depend.h"

Depend::Depend(alpm_depend_t *dependData) :
    m_alpmData(dependData)
{
}

QString Depend::name() const
{
    return m_alpmData->name;
}

QString Depend::version() const
{
    return m_alpmData->version;
}

QString Depend::description() const
{
    return m_alpmData->desc;
}

QString Depend::mod() const
{
    switch (m_alpmData->mod) {
    case ALPM_DEP_MOD_EQ:
        return " = ";
    case ALPM_DEP_MOD_GE:
        return " >= ";
    case ALPM_DEP_MOD_LE:
        return " <= ";
    case ALPM_DEP_MOD_GT:
        return " > ";
    case ALPM_DEP_MOD_LT:
        return " < ";
    default:
        return "";
    }
}
