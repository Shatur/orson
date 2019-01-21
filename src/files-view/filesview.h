#ifndef FILESVIEW_H
#define FILESVIEW_H

#include "filesmodel.h"

#include <QTreeView>
#include <QMenu>

class FilesView : public QTreeView
{
public:
    explicit FilesView(QWidget *parent = nullptr);
    FilesModel *model() const;

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
    void setModel(QAbstractItemModel *model) override;

    QMenu *m_menu;
};

#endif // FILESVIEW_H
