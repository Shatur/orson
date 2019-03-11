#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "appsettings.h"
#include "singleapplication.h"

#include <QNetworkProxy>
#include <QFileDialog>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    connect(ui->dialogButtonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, this, &SettingsDialog::restoreDefaults);

    // Setup shortcuts widget
    ui->shortcutsTreeWidget->expandAll();
    ui->shortcutsTreeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // About info version
    ui->logoLabel->setPixmap(QIcon::fromTheme("system-software-installer").pixmap(512, 512));
    ui->versionLabel->setText(SingleApplication::applicationVersion());

    // Set autosync radio buttons IDs
    ui->autosyncButtonGroup->setId(ui->noAutosyncRadioButton, AutosyncTimer::NoAutosync);
    ui->autosyncButtonGroup->setId(ui->autosyncIntervalRadioButton, AutosyncTimer::Interval);
    ui->autosyncButtonGroup->setId(ui->autosyncTimeRadioButton, AutosyncTimer::SpecifiedTime);

    loadSettings();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_proxyTypeComboBox_currentIndexChanged(int index)
{
    if (index == QNetworkProxy::HttpProxy || index == QNetworkProxy::Socks5Proxy) {
        ui->proxyHostEdit->setEnabled(true);
        ui->proxyHostLabel->setEnabled(true);
        ui->proxyPortLabel->setEnabled(true);
        ui->proxyPortSpinbox->setEnabled(true);
        ui->proxyAuthCheckBox->setEnabled(true);
    } else {
        ui->proxyHostEdit->setEnabled(false);
        ui->proxyHostLabel->setEnabled(false);
        ui->proxyPortLabel->setEnabled(false);
        ui->proxyPortSpinbox->setEnabled(false);
        ui->proxyAuthCheckBox->setEnabled(false);
    }
}

void SettingsDialog::on_loadingIconEditButton_clicked()
{
    chooseIcon(ui->loadingIconEdit);
}

void SettingsDialog::on_updatesAvailableIconButton_clicked()
{
    chooseIcon(ui->updatesAvailableIconEdit);
}

void SettingsDialog::on_noUpdatesIconButton_clicked()
{
    chooseIcon(ui->noUpdatesIconEdit);
}

void SettingsDialog::on_loadingIconEdit_textChanged(const QString &fileName)
{
    showIconPreview(ui->loadingIconPreviewLabel, fileName);
}

void SettingsDialog::on_updatesAvailableIconEdit_textChanged(const QString &fileName)
{
    showIconPreview(ui->updatesAvailableIconPreviewLabel, fileName);
}

void SettingsDialog::on_noUpdatesIconEdit_textChanged(const QString &fileName)
{
    showIconPreview(ui->noUpdatesIconPreviewLabel, fileName);
}

void SettingsDialog::on_terminalComboBox_currentTextChanged(const QString &terminalName)
{
    const AppSettings settings;
    ui->terminalArgumentsEdit->setText(settings.terminalArguments(terminalName).join(" "));

    // Set icon
    if (QIcon::hasThemeIcon(terminalName))
        ui->terminalIconLabel->setPixmap(QIcon::fromTheme(terminalName).pixmap(24, 24));
    else
        ui->terminalIconLabel->setPixmap(QIcon::fromTheme("terminal").pixmap(24, 24));
}

void SettingsDialog::on_autosyncButtonGroup_buttonToggled(QAbstractButton *button, bool checked)
{
    if (!checked)
        return;

    if (button == ui->noAutosyncRadioButton) {
        ui->autosyncTimeEdit->setEnabled(false);
        ui->autosyncIntervalSpinBox->setEnabled(false);
    } else if (button == ui->autosyncTimeRadioButton) {
        ui->autosyncTimeEdit->setEnabled(true);
        ui->autosyncIntervalSpinBox->setEnabled(false);
    } else if (button == ui->autosyncIntervalRadioButton) {
        ui->autosyncTimeEdit->setEnabled(false);
        ui->autosyncIntervalSpinBox->setEnabled(true);
    }
}

void SettingsDialog::on_shortcutsTreeWidget_itemSelectionChanged()
{
    if (ui->shortcutsTreeWidget->currentItem()->childCount() == 0) {
        ui->shortcutGroupBox->setEnabled(true);
    } else {
        ui->shortcutGroupBox->setEnabled(false);
        ui->shortcutSequenceEdit->clear();
    }

    ui->shortcutSequenceEdit->setKeySequence(ui->shortcutsTreeWidget->currentItem()->text(1));
    ui->acceptShortcutButton->setEnabled(false);
}

void SettingsDialog::on_shortcutSequenceEdit_editingFinished()
{
    if (ui->shortcutsTreeWidget->currentItem()->text(1) != ui->shortcutSequenceEdit->keySequence().toString())
        ui->acceptShortcutButton->setEnabled(true);
    else
        ui->acceptShortcutButton->setEnabled(false);
}

void SettingsDialog::on_acceptShortcutButton_clicked()
{
    ui->shortcutsTreeWidget->currentItem()->setText(1, ui->shortcutSequenceEdit->keySequence().toString());
    ui->acceptShortcutButton->setEnabled(false);
}

void SettingsDialog::on_clearShortcutButton_clicked()
{
    ui->shortcutSequenceEdit->clear();
    ui->acceptShortcutButton->setEnabled(true);
}

void SettingsDialog::on_resetShortcutButton_clicked()
{
    ui->shortcutsTreeWidget->currentItem()->setText(1, ui->shortcutsTreeWidget->currentItem()->data(1, Qt::UserRole).toString());
    ui->shortcutSequenceEdit->setKeySequence(ui->shortcutsTreeWidget->currentItem()->text(1));
    ui->acceptShortcutButton->setEnabled(false);
}

void SettingsDialog::on_resetAllShortcutsButton_clicked()
{
    QTreeWidgetItemIterator it(ui->shortcutsTreeWidget);
    while (*it) {
        QTreeWidgetItem *item = *it;
        item->setText(1, item->data(1, Qt::UserRole).toString());
        ++it;
    }
}

void SettingsDialog::on_SettingsDialog_accepted()
{
    // General settings
    AppSettings settings;
    settings.setMinimizeToTray(ui->minimizeToTrayCheckBox->isChecked());
    settings.setStartMinimized(ui->startMinimizedCheckBox->isChecked());
    settings.setAutostartEnabled(ui->autostartCheckBox->isChecked());

    // Pacman settings
    settings.setTerminal(ui->terminalComboBox->currentText());
    settings.setTerminalArguments(ui->terminalComboBox->currentText(), ui->terminalArgumentsEdit->text().split(" "));
    settings.setPacmanTool(ui->pacmanToolComboBox->currentText());
    settings.setAutosyncType(static_cast<AutosyncTimer::AutosyncType>(ui->autosyncButtonGroup->checkedId()));
    settings.setAutosyncTime(ui->autosyncTimeEdit->time());
    settings.setAutosyncInterval(ui->autosyncIntervalSpinBox->value());

    // Interface settings
    settings.setTrayIconName(PackagesModel::Loading, ui->loadingIconEdit->text());
    settings.setTrayIconName(PackagesModel::NoUpdates, ui->noUpdatesIconEdit->text());
    settings.setTrayIconName(PackagesModel::UpdatesAvailable, ui->updatesAvailableIconEdit->text());

    // Connection settings
    settings.setProxyType(static_cast<QNetworkProxy::ProxyType>(ui->proxyTypeComboBox->currentIndex()));
    settings.setProxyHost(ui->proxyHostEdit->text());
    settings.setProxyPort(static_cast<quint16>(ui->proxyPortSpinbox->value()));
    settings.setProxyAuthEnabled(ui->proxyAuthCheckBox->isChecked());
    settings.setProxyUsername(ui->proxyUsernameEdit->text());
    settings.setProxyPassword(ui->proxyPasswordEdit->text());

    // Shortcuts
    settings.setChangeModeShortcut(ui->shortcutsTreeWidget->topLevelItem(0)->text(1));
}


void SettingsDialog::restoreDefaults()
{
    // General settings
    const AppSettings settings;
    ui->minimizeToTrayCheckBox->setChecked(true);
    ui->startMinimizedCheckBox->setChecked(false);
    ui->autostartCheckBox->setChecked(false);

    // Pacman settings
    ui->terminalComboBox->setCurrentIndex(0);
    ui->pacmanToolComboBox->setCurrentIndex(0);
    ui->autosyncGroupBox->setChecked(true);
    ui->autosyncButtonGroup->buttons().at(AppSettings::defaultAutosyncType())->setChecked(true);
    ui->autosyncTimeEdit->setTime(AppSettings::defaultAutosyncTime());
    ui->autosyncIntervalSpinBox->setValue(AppSettings::defaultAutosyncInterval());

    // Interface settings
    ui->loadingIconEdit->setText(AppSettings::defaultTrayIconName(PackagesModel::Loading));
    ui->updatesAvailableIconEdit->setText(AppSettings::defaultTrayIconName(PackagesModel::UpdatesAvailable));
    ui->noUpdatesIconEdit->setText(AppSettings::defaultTrayIconName(PackagesModel::NoUpdates));

    // Connection settings
    ui->proxyTypeComboBox->setCurrentIndex(1);
    ui->proxyHostEdit->setText("");
    ui->proxyPortSpinbox->setValue(8080);
    ui->proxyAuthCheckBox->setChecked(false);
    ui->proxyUsernameEdit->setText("");
    ui->proxyPasswordEdit->setText("");

    // Shortcuts
    on_resetAllShortcutsButton_clicked();
}

void SettingsDialog::loadSettings()
{
    const AppSettings settings;

    // General settings
    ui->minimizeToTrayCheckBox->setChecked(settings.isMinimizeToTray());
    ui->startMinimizedCheckBox->setChecked(settings.isStartMinimized());
    ui->autostartCheckBox->setChecked(settings.isAutostartEnabled());

    // Pacman settings
    // Terminals
    ui->terminalComboBox->addItems(settings.availableTerminals());
    const QString terminal = settings.terminal();
    const int terminalIndex = ui->terminalComboBox->findText(terminal);
    if (terminalIndex == -1)
        ui->terminalComboBox->setCurrentText(terminal);
    else
        ui->terminalComboBox->setCurrentIndex(terminalIndex);

    // Pacman tools
    ui->pacmanToolComboBox->addItems(settings.availablePacmanTools());
    const QString pacmanTool = settings.pacmanTool();
    const int pacmanToolIndex = ui->pacmanToolComboBox->findText(pacmanTool);
    if (pacmanToolIndex == -1)
        ui->pacmanToolComboBox->setCurrentText(pacmanTool);
    else
        ui->pacmanToolComboBox->setCurrentIndex(pacmanToolIndex);

    // Autosync
    ui->autosyncButtonGroup->button(settings.autosyncType())->setChecked(true);
    ui->autosyncTimeEdit->setTime(settings.autosyncTime());
    ui->autosyncIntervalSpinBox->setValue(settings.autosyncInterval());

    // Interface settings
    ui->loadingIconEdit->setText(settings.trayIconName(PackagesModel::Loading));
    ui->updatesAvailableIconEdit->setText(settings.trayIconName(PackagesModel::UpdatesAvailable));
    ui->noUpdatesIconEdit->setText(settings.trayIconName(PackagesModel::NoUpdates));

    // Connection settings
    ui->proxyTypeComboBox->setCurrentIndex(settings.proxyType());
    ui->proxyHostEdit->setText(settings.proxyHost());
    ui->proxyPortSpinbox->setValue(settings.proxyPort());
    ui->proxyAuthCheckBox->setChecked(settings.isProxyAuthEnabled());
    ui->proxyUsernameEdit->setText(settings.proxyUsername());
    ui->proxyPasswordEdit->setText(settings.proxyPassword());

    // Shortcuts
    ui->shortcutsTreeWidget->topLevelItem(0)->setText(1, settings.changeModeShortcut());
    ui->shortcutsTreeWidget->topLevelItem(0)->setData(1, Qt::UserRole, settings.defaultChangeModeShortcut());
}

void SettingsDialog::chooseIcon(QLineEdit *iconPathEdit)
{
    QFileDialog dialog(this, tr("Select icon"));
    dialog.setNameFilter(tr("Images (*.png *.jpg *.bmp);;All files(*)"));
    dialog.setDirectory(QDir::homePath());
    dialog.setFileMode(QFileDialog::ExistingFile);

    if (!dialog.exec())
        return;

    iconPathEdit->setText(dialog.selectedFiles().at(0));
}

void SettingsDialog::showIconPreview(QLabel *previewLabel, const QString &fileName)
{
    if (QIcon::hasThemeIcon(fileName))
        previewLabel->setPixmap(QIcon::fromTheme(fileName).pixmap(24, 24));
    else
        previewLabel->setPixmap(QIcon(fileName).pixmap(24, 24));
}
