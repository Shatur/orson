#include "searchedit.h"
#include "appsettings.h"

#include <QTimer>

SearchEdit::SearchEdit(QWidget *parent) :
    QLineEdit(parent)
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &SearchEdit::processTextChanged);
    setInstantSearchEnabled(AppSettings::defaultIsInstantSearchEnabled());
}

void SearchEdit::setInstantSearchEnabled(bool enabled)
{
    if (enabled) {
        connect(this, &SearchEdit::textChanged, this, &SearchEdit::waitEditInterval);
        disconnect(this, &SearchEdit::returnPressed, this, &SearchEdit::processTextChanged);
    } else {
        disconnect(this, &SearchEdit::textChanged, this, &SearchEdit::waitEditInterval);
        connect(this, &SearchEdit::returnPressed, this, &SearchEdit::processTextChanged);
    }
}

void SearchEdit::waitEditInterval()
{
    m_timer->start(200);
}

void SearchEdit::processTextChanged()
{
    emit textSearched(text());
}
