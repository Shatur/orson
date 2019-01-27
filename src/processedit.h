#ifndef PROCESSEDIT_H
#define PROCESSEDIT_H

#include <QTextEdit>

class ProcessEdit : public QTextEdit
{
    Q_OBJECT

public:
    ProcessEdit(QWidget *parent = nullptr);

public slots:
    void addProcessData(const QString &text);

private:
    QString getCurrentProcessInfo();
    QStringList formatProcessData(QString text);
    void replaceLastLine(const QString &line);
};

#endif // PROCESSEDIT_H
