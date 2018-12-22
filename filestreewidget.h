#ifndef FILESTREEWIDGET_H
#define FILESTREEWIDGET_H

#include <QTreeWidget>

class FilesTreeWidget : public QTreeWidget
{
public:
    FilesTreeWidget(QWidget *parent = nullptr);

    void addPath(const QString &path);

private slots:
    void open(QTreeWidgetItem *item);
};

#endif // FILESTREEWIDGET_H
