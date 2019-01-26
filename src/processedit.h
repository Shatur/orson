#ifndef PROCESSEDIT_H
#define PROCESSEDIT_H

#include <QPlainTextEdit>

class ProcessEdit : public QPlainTextEdit
{
    Q_OBJECT

public:
    ProcessEdit(QWidget *parent = nullptr);

public slots:
    void addProcessData(const QString &text);

private:
    QString getCurrentProcessInfo();
    void replaceLastLine(const QString &line);

};

#endif // PROCESSEDIT_H
