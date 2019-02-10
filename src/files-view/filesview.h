#ifndef FILESVIEW_H
#define FILESVIEW_H

#include <QTreeView>

class FilesModel;

class FilesView : public QTreeView
{
public:
    explicit FilesView(QWidget *parent = nullptr);
    FilesModel *model() const;

private slots:
    // Context menu actions
    void openCurrent() const;
    void openCurrentInFilesystem() const;
    void copyCurrentFile() const;
    void copyCurrentName() const;
    void copyCurrentPath() const;

private:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent * event) override;
    void setModel(QAbstractItemModel *model) override;

    QMenu *m_menu;
};

#endif // FILESVIEW_H
