#ifndef FILESTREEWIDGET_H
#define FILESTREEWIDGET_H

#include <QTreeWidget>
#include <QMenu>

class FilesTreeWidget : public QTreeWidget
{
public:
    FilesTreeWidget(QWidget *parent = nullptr);

    void addPath(const QString &path);

private slots:
    // Context menu actions
    void open();
    void openInFileManager();
    void copyFile();
    void copyName();
    void copyPath();

private:
    void contextMenuEvent(QContextMenuEvent *event) override;

    QMenu menu{this};
};

#endif // FILESTREEWIDGET_H
