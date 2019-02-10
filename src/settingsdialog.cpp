/*
 *  Copyright © 2018-2019 Hennadii Chernyshchyk <genaloner@gmail.com>
 *
 *  This file is part of Crow Translate.
 *
 *  Crow Translate is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a get of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include "appsettings.h"
#include "singleapplication.h"

#include <QNetworkProxy>
#include <QFileDialog>
#include <QScreen>
#include <QMessageBox>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    ui->shortcutsTreeWidget->expandAll();
    ui->shortcutsTreeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->logoLabel->setPixmap(QIcon::fromTheme("system-software-installer").pixmap(512, 512));
    ui->versionLabel->setText(SingleApplication::applicationVersion());

    // General settings
    const AppSettings settings;
    ui->trayCheckBox->setChecked(settings.isTrayIconVisible());
    ui->startMinimizedCheckBox->setChecked(settings.isStartMinimized());
    ui->autostartCheckBox->setChecked(settings.isAutostartEnabled());

    // Interface settings
    ui->noUpdatesIconEdit->setText(settings.trayIconName(MainWindow::NoUpdates));
    ui->updatingIconEdit->setText(settings.trayIconName(MainWindow::Updating));
    ui->updatesAvailableIconEdit->setText(settings.trayIconName(MainWindow::UpdatesAvailable));

    // Connection settings
    ui->proxyTypeComboBox->setCurrentIndex(settings.proxyType());
    ui->proxyHostEdit->setText(settings.proxyHost());
    ui->proxyPortSpinbox->setValue(settings.proxyPort());
    ui->proxyAuthCheckBox->setChecked(settings.isProxyAuthEnabled());
    ui->proxyUsernameEdit->setText(settings.proxyUsername());
    ui->proxyPasswordEdit->setText(settings.proxyPassword());
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_SettingsDialog_accepted()
{
    // General settings
    AppSettings settings;
    settings.setTrayIconVisible(ui->trayCheckBox->isChecked());
    settings.setStartMinimized(ui->startMinimizedCheckBox->isChecked());
    settings.setAutostartEnabled(ui->autostartCheckBox->isChecked());

    // Interface settings
    settings.setTrayIconName(MainWindow::NoUpdates, ui->noUpdatesIconEdit->text());
    settings.setTrayIconName(MainWindow::Updating, ui->updatingIconEdit->text());
    settings.setTrayIconName(MainWindow::UpdatesAvailable, ui->updatesAvailableIconEdit->text());

    // Connection settings
    settings.setProxyType(static_cast<QNetworkProxy::ProxyType>(ui->proxyTypeComboBox->currentIndex()));
    settings.setProxyHost(ui->proxyHostEdit->text());
    settings.setProxyPort(static_cast<quint16>(ui->proxyPortSpinbox->value()));
    settings.setProxyAuthEnabled(ui->proxyAuthCheckBox->isChecked());
    settings.setProxyUsername(ui->proxyUsernameEdit->text());
    settings.setProxyPassword(ui->proxyPasswordEdit->text());
}

void SettingsDialog::on_resetSettingsButton_clicked()
{
    // General settings
    const AppSettings settings;
    ui->trayCheckBox->setChecked(true);
    ui->startMinimizedCheckBox->setChecked(false);
    ui->autostartCheckBox->setChecked(false);

    // Interface settings
    ui->noUpdatesIconEdit->setText(settings.defaultTrayIconName(MainWindow::NoUpdates));
    ui->updatingIconEdit->setText(settings.defaultTrayIconName(MainWindow::Updating));
    ui->updatesAvailableIconEdit->setText(settings.defaultTrayIconName(MainWindow::UpdatesAvailable));

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

void SettingsDialog::on_noUpdatesIconButton_clicked()
{
    chooseIcon(ui->noUpdatesIconEdit);
}

void SettingsDialog::on_updatingIconEditButton_clicked()
{
    chooseIcon(ui->updatingIconEdit);
}

void SettingsDialog::on_updatesAvailableIconButton_clicked()
{
    chooseIcon(ui->updatesAvailableIconEdit);
}

void SettingsDialog::on_noUpdatesIconEdit_textChanged(const QString &fileName)
{
    showIconPreview(ui->noUpdatesIconPreviewLabel, fileName);
}

void SettingsDialog::on_updatingIconEdit_textChanged(const QString &fileName)
{
    showIconPreview(ui->updatingIconPreviewLabel, fileName);
}

void SettingsDialog::on_updatesAvailableIconEdit_textChanged(const QString &fileName)
{
    showIconPreview(ui->updatesAvailableIconPreviewLabel, fileName);
}

void SettingsDialog::on_shortcutsTreeWidget_itemSelectionChanged()
{
    if (ui->shortcutsTreeWidget->currentItem()->childCount() == 0) {
        ui->shortcutGroupBox->setEnabled(true);
        ui->shortcutSequenceEdit->setKeySequence(ui->shortcutsTreeWidget->currentItem()->text(1));
    } else {
        ui->shortcutGroupBox->setEnabled(false);
        ui->shortcutSequenceEdit->clear();
    }

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
    QTreeWidgetItemIterator it(ui->shortcutsTreeWidget, QTreeWidgetItemIterator::NoChildren);
    while (*it) {
        QTreeWidgetItem *item = *it;
        item->setText(1, item->data(1, Qt::UserRole).toString());
        ++it;
    }
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
