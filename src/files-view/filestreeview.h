#ifndef FILESTREEVIEW_H
#define FILESTREEVIEW_H

#include <QTreeView>
#include <QMenu>

class FilesTreeView : public QTreeView
{
public:
    FilesTreeView(QWidget *parent = nullptr);

private slots:
    // Context menu actions
    void open() const;
    void openInFileManager() const;
    void copyFile() const;
    void copyName() const;
    void copyPath() const;

private:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent * event) override;

    QMenu menu{this};
};

#endif // FILESTREEVIEW_H
