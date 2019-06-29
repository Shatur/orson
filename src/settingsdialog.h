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
    Q_DISABLE_COPY(SettingsDialog)

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog() override;

public slots:
    void accept() override;

private slots:
    void chooseLoadingIcon();
    void chooseUpdatesAvailableIcon();
    void chooseNoUpdatesIcon();
    void setLoadingIcon(const QString &fileName);
    void setUpdatesAvailableIcon(const QString &fileName);
    void setNoUpdatesIcon(const QString &fileName);

    void on_terminalComboBox_currentTextChanged(const QString &terminalName);
    void on_autosyncButtonGroup_buttonToggled(QAbstractButton *button, bool checked);

    void on_proxyTypeComboBox_currentIndexChanged(int index);

    void on_shortcutsTreeWidget_itemSelectionChanged();
    void on_shortcutSequenceEdit_editingFinished();
    void on_acceptShortcutButton_clicked();
    void on_clearShortcutButton_clicked();
    void on_resetShortcutButton_clicked();
    void on_resetAllShortcutsButton_clicked();

    void restoreDefaults();

private:
    void loadSettings();
    QString chooseIcon();

    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
