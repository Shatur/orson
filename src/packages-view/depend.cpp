#include "depend.h"

Depend::Depend(alpm_depend_t *dependData)
{
    m_name = dependData->name;
    m_version = dependData->version;

    switch (dependData->mod) {
    case ALPM_DEP_MOD_EQ:
        m_mod = " = ";
        break;
    case ALPM_DEP_MOD_GE:
        m_mod = " >= ";
        break;
    case ALPM_DEP_MOD_LE:
        m_mod = " <= ";
        break;
    case ALPM_DEP_MOD_GT:
        m_mod = " > ";
        break;
    case ALPM_DEP_MOD_LT:
        m_mod = " < ";
        break;
    default:
        m_mod = "";
    }
}

Depend::Depend(const QString &text)
{
    int index = text.indexOf(">");
    if (index != -1) {
        parseDepend(text, ">", index);
        return;
    }

    index = text.indexOf("<");
    if (index != -1) {
        parseDepend(text, "<", index);
        return;
    }

    index = text.indexOf("=");
    if (index != -1) {
        parseDepend(text, "=", index);
        return;
    }

    index = text.indexOf(">=");
    if (index != -1) {
        parseDepend(text, ">=", index);
        return;
    }

    index = text.indexOf("<=");
    if (index != -1) {
        parseDepend(text, "<=", index);
        return;
    }

    // Just copy all text if unable to parse
    m_name = text;
}

QString Depend::name() const
{
    return m_name;
}

QString Depend::version() const
{
    return m_version;
}

QString Depend::description() const
{
    return m_description;
}

QString Depend::mod() const
{
    return m_mod;;
}

void Depend::parseDepend(const QString &text, const QString &mod, int position)
{
    const int modSize = mod.size();

    // Tokenize string like "jre>=11" to "jre", ">=" and "11"
    m_name = text.left(position);
    m_mod = text.mid(position, position + modSize);
    m_version = text.right(position + modSize);
}
