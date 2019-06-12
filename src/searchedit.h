#ifndef SEARCHEDIT_H
#define SEARCHEDIT_H

#include <QLineEdit>

class SearchEdit : public QLineEdit
{
    Q_OBJECT
    Q_DISABLE_COPY(SearchEdit)

public:
    SearchEdit(QWidget *parent = nullptr);

    void setInstantSearchEnabled(bool enabled);

signals:
    void textSearched(const QString &searchText);

private slots:
    void waitEditInterval();
    void processTextChanged();

private:
    QTimer *m_timer;
};

#endif // SEARCHEDIT_H
