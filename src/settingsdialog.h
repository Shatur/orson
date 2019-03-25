#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class QAbstractButton;

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
    void on_loadingIconButton_clicked();
    void on_updatesAvailableIconButton_clicked();
    void on_noUpdatesIconButton_clicked();
    void on_loadingIconEdit_textChanged(const QString &fileName);
    void on_updatesAvailableIconEdit_textChanged(const QString &fileName);
    void on_noUpdatesIconEdit_textChanged(const QString &fileName);

    void on_terminalComboBox_currentTextChanged(const QString &terminalName);
    void on_autosyncButtonGroup_buttonToggled(QAbstractButton *button, bool checked);

    void on_proxyTypeComboBox_currentIndexChanged(int index);

    void on_shortcutsTreeWidget_itemSelectionChanged();
    void on_shortcutSequenceEdit_editingFinished();
    void on_acceptShortcutButton_clicked();
    void on_clearShortcutButton_clicked();
    void on_resetShortcutButton_clicked();
    void on_resetAllShortcutsButton_clicked();

    void on_SettingsDialog_accepted();
    void restoreDefaults();

private:
    void loadSettings();
    QString chooseIcon();

    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
