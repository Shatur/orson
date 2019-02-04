#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QMenu>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

private slots:
    // UI
    void on_SettingsDialog_accepted();
    void on_resetSettingsButton_clicked();
    void on_proxyTypeComboBox_currentIndexChanged(int index);

    void on_shortcutsTreeWidget_itemSelectionChanged();
    void on_shortcutSequenceEdit_editingFinished();
    void on_acceptShortcutButton_clicked();
    void on_clearShortcutButton_clicked();
    void on_resetShortcutButton_clicked();
    void on_resetAllShortcutsButton_clicked();

private:
    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
