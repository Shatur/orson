#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tasksdialog.h"
#include "autosynctimer.h"
#include "appsettings.h"
#include "pacmansettings.h"
#include "settingsdialog.h"
#include "packages-view/package.h"
#include "packages-view/packagesmodel.h"
#include "files-view/filesmodel.h"
#include "singleapplication.h"

#include <QPushButton>
#include <QStandardItemModel>
#include <QDesktopServices>
#include <QMessageBox>
#include <QFileDialog>
#include <QDBusInterface>
#include <QButtonGroup>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->packagesView->model(), &PackagesModel::databaseStatusChanged, this, &MainWindow::setStatusBarMessage);
    connect(ui->packagesView->model(), &PackagesModel::firstPackageAvailable, this, &MainWindow::processFirstPackageAvailable);
    connect(ui->packagesView->model(), &PackagesModel::databaseLoaded, this, &MainWindow::processLoadedDatabase);
    connect(ui->packagesView, &PackagesView::operationsCountChanged, this, &MainWindow::processOperationsCountChanged);

    // Setup terminal
    m_pacman = new Pacman(this);
    m_pacman->setTasks(ui->packagesView);
    connect(m_pacman, &Pacman::finished, this, &MainWindow::processTerminalFinish);
    connect(m_pacman, &Pacman::started, this, &MainWindow::processTerminalStart);

    // Setup autosync
    m_autosyncTimer = new AutosyncTimer(this);
    connect(m_autosyncTimer, &AutosyncTimer::timeout, m_pacman, &Pacman::syncDatabase); // Automatically sync databases in background

    // Make after completion actions exclusive
    m_afterCompletionGroup = new QActionGroup(this);
    m_afterCompletionGroup->addAction(ui->waitForInputAction);
    m_afterCompletionGroup->addAction(ui->shutdownAction);
    m_afterCompletionGroup->addAction(ui->rebootAction);
    connect(m_afterCompletionGroup, &QActionGroup::triggered, this, &MainWindow::setAfterTasksCompletionAction);

    // Select package when clicking on dependencies
    m_depsButtonGroup = new QButtonGroup(this);
    connect(m_depsButtonGroup, qOverload<QAbstractButton *>(&QButtonGroup::buttonClicked), this, &MainWindow::findDepend);

    // System tray menu
    m_trayMenu = new QMenu(this);
    m_trayMenu->addAction(QIcon::fromTheme("window"), tr("Show window"), this, &MainWindow::show);
    m_trayMenu->addAction(ui->settingsAction->icon(), ui->settingsAction->iconText(), this, &MainWindow::on_settingsAction_triggered);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(QIcon::fromTheme("system-upgrade"), tr("Sync and upgrade"), this, &MainWindow::syncAndUpgrade);
    m_trayMenu->addAction(ui->upgradeButton->icon(), ui->upgradeButton->text(), this, &MainWindow::upgrade);
    m_trayMenu->addAction(ui->syncButton->icon(), ui->syncButton->text(), this, &MainWindow::syncRepositories);
    m_trayMenu->addAction(ui->reloadButton->icon(), ui->reloadButton->text(), this, &MainWindow::on_reloadButton_clicked);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(QIcon::fromTheme("application-exit"), tr("Exit"), SingleApplication::instance(), &SingleApplication::quit);

    // System tray icon
#ifdef KDE
    m_trayIcon = new KStatusNotifierItem(this);
    m_trayIcon->setStandardActionsEnabled(false);
    m_trayIcon->setToolTipTitle(SingleApplication::applicationName());
    m_trayIcon->setToolTipIconByName(windowIcon().name());
    m_trayIcon->setCategory(KStatusNotifierItem::SystemServices);
    connect(m_trayIcon, &KStatusNotifierItem::secondaryActivateRequested, &KStatusNotifierItem::activate);
#else
    m_trayIcon = new QSystemTrayIcon(this);
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::activateTray);
#endif
    m_trayIcon->setContextMenu(m_trayMenu);

    // Select the first package if it is not already selected (when first package loaded faster than window)
    if (ui->packagesView->model()->index(0, 0).isValid() && ui->packagesView->selectionModel()->selectedRows().isEmpty())
        processFirstPackageAvailable();

    // Load app settings
    loadMainWindowSettings();
    loadAppSettings();
}

MainWindow::~MainWindow()
{
    // Save window settings
    AppSettings settings;
    settings.setNoConfirm(ui->noConfirmAction->isChecked());
    settings.setMainWindowGeometry(saveGeometry());

    QAction *afterCompletionAction = m_afterCompletionGroup->checkedAction();
    const auto afterCompletion = static_cast<Pacman::AfterCompletion>(m_afterCompletionGroup->actions().indexOf(afterCompletionAction));
    settings.setAfterCompletion(afterCompletion);

    delete ui;
}

void MainWindow::on_installLocalAction_triggered()
{
    QFileDialog dialog(this, tr("Select package"));
    dialog.setNameFilter(tr("Pacman package (*.pkg.tar.xz);;All files(*)"));
    dialog.setDirectory(QDir::homePath());
    dialog.setFileMode(QFileDialog::ExistingFile);

    if (!dialog.exec())
        return;

    m_pacman->installLocalPackage(dialog.selectedFiles().at(0));
}

void MainWindow::on_installLocalDependAction_triggered()
{
    QFileDialog dialog(this, tr("Select package"));
    dialog.setNameFilter(tr("Pacman package (*.pkg.tar.xz);;All files(*)"));
    dialog.setDirectory(QDir::homePath());
    dialog.setFileMode(QFileDialog::ExistingFile);

    if (!dialog.exec())
        return;

    m_pacman->installLocalPackage(dialog.selectedFiles().at(0), true);
}

void MainWindow::on_exitAction_triggered()
{
    SingleApplication::exit();
}

void MainWindow::on_noConfirmAction_toggled(bool checked)
{
    m_pacman->setNoConfirm(checked);
}

void MainWindow::on_forceAction_toggled(bool checked)
{
    m_pacman->setForce(checked);
}

void MainWindow::on_openHistoryFileAction_triggered()
{
    const PacmanSettings pacmanSettings;
    const QFileInfo logFile = pacmanSettings.logFile();
    QDesktopServices::openUrl(logFile.filePath());
}

void MainWindow::on_openHistoryFolderAction_triggered()
{
    const PacmanSettings pacmanSettings;
    const QFileInfo logFile = pacmanSettings.logFile();
    QDesktopServices::openUrl(logFile.dir().path());
}

void MainWindow::on_settingsAction_triggered()
{
    SettingsDialog dialog;
    if (dialog.exec())
        loadAppSettings();
}

void MainWindow::setAfterTasksCompletionAction(QAction *action)
{
    auto afterCompletion = static_cast<Pacman::AfterCompletion>(m_afterCompletionGroup->actions().indexOf(action));
    m_pacman->setAfterTasksCompletion(afterCompletion);
}

void MainWindow::syncAndUpgrade()
{
    ui->packagesView->setSyncRepositories(true);
    ui->packagesView->setUpgradePackages(true);
    on_applyButton_clicked();
}

void MainWindow::syncRepositories()
{
    ui->packagesView->setSyncRepositories(true);
    on_applyButton_clicked();
}

void MainWindow::upgrade()
{
    ui->packagesView->setUpgradePackages(true);
    on_applyButton_clicked();
}

void MainWindow::on_applyButton_clicked()
{
    TasksDialog dialog(m_pacman, ui->packagesView, ui->menuBar, this);
    if (dialog.exec())
        m_pacman->executeTasks();

    // Set pacman options back (thay may be changed in dialog)
    on_noConfirmAction_toggled(ui->noConfirmAction->isChecked());
    on_forceAction_toggled(ui->forceAction->isChecked());
    setAfterTasksCompletionAction(m_afterCompletionGroup->checkedAction());
}

void MainWindow::on_syncButton_toggled(bool checked)
{
    ui->packagesView->setSyncRepositories(checked);

    if (checked) {
        ui->upgradeButton->setEnabled(true); // Enable upgrade button even if no updates available
    } else if (ui->packagesView->model()->outdatedPackages().isEmpty()) {
        ui->upgradeButton->setChecked(false);
        ui->upgradeButton->setEnabled(false);
    }
}

void MainWindow::on_upgradeButton_toggled(bool checked)
{
    ui->packagesView->setUpgradePackages(checked);
}

void MainWindow::on_reloadButton_clicked()
{
    ui->reloadButton->setEnabled(false);
    ui->syncButton->setEnabled(false);
    ui->upgradeButton->setEnabled(false);

    ui->syncButton->setChecked(false);
    ui->upgradeButton->setChecked(false);

    setTrayStatus(Updating);

    ui->packagesView->model()->reloadRepoPackages();
}

void MainWindow::on_browserButton_clicked()
{
    QUrl url;
    const Package *package = ui->packagesView->currentPackage();
    if (package->repo() == "aur")
        url = "https://aur.archlinux.org/packages/" + package->name();
    else
        url = "https://www.archlinux.org/packages/" + package->repo() + "/" + package->arch() + "/" + package->name();

    QDesktopServices::openUrl(url);
}

void MainWindow::on_searchModeComboBox_currentIndexChanged(int index)
{
    const auto mode = static_cast<PackagesModel::Mode>(index);

    // Disable search by description for AUR
    if (mode == PackagesModel::AUR) {
        qobject_cast<QStandardItemModel *>(ui->searchByComboBox->model())->item(3)->setEnabled(false);
        if (ui->searchByComboBox->currentIndex() == 3)
            ui->searchByComboBox->setCurrentIndex(0);
    } else {
        qobject_cast<QStandardItemModel *>(ui->searchByComboBox->model())->item(3)->setEnabled(true);
    }

    ui->packagesView->model()->setMode(mode);
    on_searchPackagesEdit_returnPressed(); // Search packages
}

void MainWindow::on_searchPackagesEdit_returnPressed()
{
    const auto filterType = static_cast<PackagesView::FilterType>(ui->searchByComboBox->currentIndex());
    ui->packagesView->filter(ui->searchPackagesEdit->text(), filterType);
}

void MainWindow::on_packagesView_currentPackageChanged(Package *package)
{
    // Reset loaded tabs information
    ui->infoTab->setProperty("loaded", false);
    ui->depsTab->setProperty("loaded", false);
    ui->filesTab->setProperty("loaded", false);

    // Load package info header
    ui->iconLabel->setPixmap(package->icon().pixmap(64, 64));
    ui->nameLabel->setText(package->name());
    ui->descriptionLabel->setText(package->description());

    // Show available update in header
    const QString availableUpdate = package->availableUpdate();
    if (availableUpdate.isEmpty())
        ui->versionLabel->setText(package->version());
    else
        ui->versionLabel->setText(R"(<span style="color:red">)" + package->version() + "</span> ⇒ " + availableUpdate);

    // Disable the tab with files for uninstalled packages
    if (package->isInstalled())
        ui->packageTabsWidget->setTabEnabled(2, true);
    else
        ui->packageTabsWidget->setTabEnabled(2, false);

    // Disable "Open in browser" button for local packages
    if (package->repo() == "local")
        ui->browserButton->setDisabled(true);
    else
        ui->browserButton->setDisabled(false);

    // Reload opened tab
    switch (ui->packageTabsWidget->currentIndex()) {
    case 0:
        loadPackageInfo(package);
        return;
    case 1:
        loadPackageDeps(package);
        return;
    case 2:
        if (package->isInstalled())
            loadPackageFiles(package);
        else
            ui->packageTabsWidget->setCurrentIndex(0);
        return;
    default:
        return;
    }
}

void MainWindow::on_packageTabsWidget_currentChanged(int index)
{
    const Package *package = ui->packagesView->currentPackage();

    switch (index) {
    case 0:
        if (!ui->infoTab->property("loaded").toBool())
            loadPackageInfo(package);
        return;
    case 1:
        if (!ui->depsTab->property("loaded").toBool())
            loadPackageDeps(package);
        return;
    case 2:
        if (!ui->filesTab->property("loaded").toBool())
            loadPackageFiles(package);
        return;
    default:
        return;
    }
}

#ifndef KDE
void MainWindow::activateTray(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        if (!this->isVisible()) {
            show();
            activateWindow();
            raise();
        } else {
            hide();
        }
    }
}
#endif

void MainWindow::showNotification(const QString &message, int interval)
{
#ifdef KDE
    m_trayIcon->showMessage(SingleApplication::applicationName(), message, windowIcon().name(), interval);
#else
    QDBusInterface notify("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications");
    QVariantList notifyArguments;
    notifyArguments << SingleApplication::applicationName(); // Set program name
    notifyArguments << QVariant(QVariant::UInt);
    notifyArguments << windowIcon().name(); // Icon
    notifyArguments << SingleApplication::applicationName(); // Title
    notifyArguments << message; // Body
    notifyArguments << QStringList();
    notifyArguments << QVariantMap();
    notifyArguments << interval; // Show interval
    notify.callWithArgumentList(QDBus::AutoDetect, "Notify", notifyArguments);
#endif
}

void MainWindow::setTrayStatus(MainWindow::TrayStatus trayStatus)
{
    m_trayStatus = trayStatus;

    // Set icon
    const AppSettings settings;
    const QString iconName = settings.trayIconName(m_trayStatus);
#ifdef KDE
    if (QIcon::hasThemeIcon(iconName))
        m_trayIcon->setIconByName(iconName);
    else if (QFile::exists(iconName))
        m_trayIcon->setIconByPixmap(QIcon(iconName));
    else
        m_trayIcon->setIconByName("dialog-error");
#else
    if (QIcon::hasThemeIcon(iconName))
        m_trayIcon->setIcon(QIcon::fromTheme(iconName));
    else if (QFile::exists(iconName))
        m_trayIcon->setIcon(QIcon(iconName));
    else
        m_trayIcon->setIcon(QIcon::fromTheme("dialog-error"));
#endif

#ifdef KDE
    const QDateTime datetime = settings.lastSync();
    QString syncString = tr("Last sync: ");
    if (datetime.isNull())
        syncString += tr("never");
    else
        syncString += datetime.toString();
#endif

    // Show notification and KDE tooltip
    switch (m_trayStatus) {
    case NoUpdates:
    {
        const QString message = tr("No updates available");
        showNotification(message + '\n' + syncString, 10000);
#ifdef KDE
        m_trayIcon->setToolTipSubTitle(message);
        m_trayIcon->setStatus(KStatusNotifierItem::Passive);
#endif
        break;
    }
    case Updating:
#ifdef KDE
        m_trayIcon->setToolTipSubTitle("Synchronizing databases");
        m_trayIcon->setStatus(KStatusNotifierItem::Active);
#endif
        break;
    case UpdatesAvailable:
    {
        const QString message = QString::number(ui->packagesView->model()->outdatedPackages().size()) + tr(" updates available");
        showNotification(message, 10000);
#ifdef KDE
        m_trayIcon->setToolTipSubTitle(message + '\n' + syncString);
        m_trayIcon->setStatus(KStatusNotifierItem::NeedsAttention);
#endif
    }
        break;
    }
}

void MainWindow::setStatusBarMessage(const QString &text)
{
    statusBar()->showMessage(text);
}

void MainWindow::findDepend(QAbstractButton *button)
{
    // Clear filter
    ui->searchPackagesEdit->clear();
    if (ui->searchModeComboBox->currentIndex() != PackagesModel::Repo)
        ui->searchModeComboBox->setCurrentIndex(PackagesModel::Repo);
    else
        on_searchPackagesEdit_returnPressed();

    // Search package in repo first
    const bool found = ui->packagesView->find(button->toolTip());
    if (!found) {
        // Search in AUR
        ui->searchPackagesEdit->setText(button->toolTip());
        ui->searchModeComboBox->setCurrentIndex(PackagesModel::AUR);
    }
}

void MainWindow::processLoadedDatabase()
{
    // Set tray status
    if (ui->packagesView->model()->outdatedPackages().isEmpty())
        setTrayStatus(NoUpdates);
    else
        setTrayStatus(UpdatesAvailable);

    // Enable buttons
    ui->reloadButton->setEnabled(true);
    ui->syncButton->setEnabled(true);

    // Enable upgrade button only if updates available
    if (ui->packagesView->model()->outdatedPackages().isEmpty())
        ui->upgradeButton->setEnabled(false);
    else
        ui->upgradeButton->setEnabled(true);
}

void MainWindow::processFirstPackageAvailable()
{
    ui->packagesView->setCurrentIndex(ui->packagesView->model()->index(0, 0));
    setStatusBarMessage("Loading installed packages");
}

void MainWindow::processOperationsCountChanged(int tasksCount)
{
    if (tasksCount > 0) {
        ui->applyButton->setToolTip("Apply tasks (" + QString::number(tasksCount) + ")");
        ui->applyButton->setEnabled(true);
    } else {
        ui->applyButton->setToolTip("Apply tasks");
        ui->applyButton->setEnabled(false);
    }

    ui->upgradeButton->setChecked(ui->packagesView->isUpgradePackages());
    ui->syncButton->setChecked(ui->packagesView->isSyncRepositories());
}

void MainWindow::processTerminalStart()
{
    setTrayStatus(Updating);
    ui->reloadButton->setEnabled(false);
}

void MainWindow::processTerminalFinish(int exitCode)
{
    if (exitCode != 0) {
        if (ui->packagesView->model()->outdatedPackages().isEmpty())
            setTrayStatus(MainWindow::NoUpdates);
        else
            setTrayStatus(MainWindow::UpdatesAvailable);
    } else {
        on_reloadButton_clicked();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Check if user disabled minimizing to tray
    AppSettings settings;
    if (!settings.isMinimizeToTray())
        SingleApplication::quit();

    QMainWindow::closeEvent(event);
}

void MainWindow::loadPackageInfo(const Package *package)
{
    // Licenses
    const QStringList licenses = package->licenses();
    displayInfo(!licenses.isEmpty(), licenses.join(", "), ui->licensesTitleLabel, ui->licensesLabel);

    // URL
    const QString url = package->url();
    displayInfo(!url.isEmpty(), QString("<a href=\"" + url + "\">" + url + "</a>"), ui->urlTitleLabel, ui->urlLabel);

    // Keywords (AUR)
    const QStringList keywords = package->keywords();
    displayInfo(!keywords.isEmpty(), keywords.join(", "), ui->keywordsTitleLabel, ui->keywordsLabel);

    // Arch
    const QString arch = package->arch();
    displayInfo(!arch.isEmpty(), arch, ui->archTitleLabel, ui->archLabel);

    // Groups
    const QStringList groups = package->groups();
    displayInfo(!groups.isEmpty(), groups.join(", "), ui->groupsTitlelabel, ui->groupsLabel);

    // Sizes
    displayInfo(package->downloadSize() != -1, package->formattedDownloadSize(), ui->downloadSizeTitleLabel, ui->downloadSizeLabel);
    displayInfo(package->installedSize() != -1, package->formattedInstalledSize(), ui->installedSizeTitleLabel, ui->installedSizeLabel);

    // Build date
    const QDateTime buildDate = package->buildDate();
    displayInfo(buildDate.isValid(), buildDate.toString("ddd dd MMM yyyy HH:mm:ss"), ui->buildDateTitleLabel, ui->buildDateLabel);

    // Install date
    const QDateTime installDate = package->installDate();
    displayInfo(installDate.isValid(), installDate.toString("ddd dd MMM yyyy HH:mm:ss"), ui->installDateTitleLabel, ui->installDateLabel);

    // First submitted date (AUR)
    const QDateTime firstSubmitted = package->firstSubmitted();
    displayInfo(firstSubmitted.isValid(), firstSubmitted.toString("ddd dd MMM yyyy HH:mm:ss"), ui->firstSubmittedTitleLabel, ui->firstSubmittedLabel);

    // Last submitted date (AUR)
    const QDateTime lastSubmitted = package->lastModified();
    displayInfo(lastSubmitted.isValid(), lastSubmitted.toString("ddd dd MMM yyyy HH:mm:ss"), ui->lastModifiedTitleLabel, ui->lastModifiedLabel);

    // Out of date (AUR)
    const QDateTime outOfDate = package->outOfDate();
    displayInfo(outOfDate.isValid(), outOfDate.toString("ddd dd MMM yyyy HH:mm:ss"), ui->outOfDateTitleLabel, ui->outOfDateLabel);

    // Maintainer
    const QString maintainer = package->maintainer();
    if (!maintainer.isEmpty())
        ui->maintainerLabel->setText(package->maintainer());
    else
        ui->maintainerLabel->setText("None");

    // Other install-specific info
    if (package->isInstalled()) {
        // Reason
        ui->reasonLabel->setVisible(true);
        ui->reasonTitleLabel->setVisible(true);
        if (package->isInstalledExplicitly())
            ui->reasonLabel->setText(tr("Installed explicitly"));
        else
            ui->reasonLabel->setText(tr("Installed as dependency"));

        // Install script
        ui->scriptLabel->setVisible(true);
        ui->scriptTitleLabel->setVisible(true);
        if (package->hasScript())
            ui->scriptLabel->setText(tr("Yes"));
        else
            ui->scriptLabel->setText(tr("No"));
    } else {
        ui->reasonLabel->setVisible(false);
        ui->reasonTitleLabel->setVisible(false);
        ui->scriptLabel->setVisible(false);
        ui->scriptTitleLabel->setVisible(false);
    }

    ui->infoTab->setProperty("loaded", true);
}

void MainWindow::loadPackageDeps(const Package *package)
{
    loadDepsButtons(0, package->provides());
    loadDepsButtons(1, package->replaces());
    loadDepsButtons(2, package->conflicts());
    loadDepsButtons(3, package->depends());
    loadDepsButtons(4, package->optdepends());

    ui->depsTab->setProperty("loaded", true);
}

void MainWindow::loadPackageFiles(const Package *package)
{
    ui->filesView->model()->setPaths(package->files());
    ui->filesTab->setProperty("loaded", true);
}

void MainWindow::displayInfo(bool display, const QString &text, QLabel *titleLabel, QLabel *label)
{
    if (display) {
        // Spacing in layout must be disabled to hide elements without empty space.
        // So need to add spacing between items dynamically
        label->setMargin(6);
        titleLabel->setMargin(6);

        label->setText(text);
        label->setVisible(true);
        titleLabel->setVisible(true);
    } else {
        label->setMargin(0);
        titleLabel->setMargin(0);

        label->setVisible(false);
        titleLabel->setVisible(false);
    }
}

void MainWindow::loadDepsButtons(int row, const QVector<Depend> &deps)
{
    auto *depsContentLayout = qobject_cast<QFormLayout *>(ui->depsContentWidget->layout());
    auto *packagesLabel = qobject_cast<QLabel *>(depsContentLayout->itemAt(row, QFormLayout::LabelRole)->widget());
    auto *packagesLayout = qobject_cast<QVBoxLayout *>(depsContentLayout->itemAt(row, QFormLayout::FieldRole)->layout());

    // Remove old items
    while (QLayoutItem *child = packagesLayout->takeAt(0)) {
        delete child->widget();
        delete child;
    }

    // Hide label if no dependencies
    if (deps.isEmpty()) {
        packagesLabel->hide();
        return;
    }

    // Add new
    foreach (const Depend &depend, deps) {
        auto *button = new QPushButton;
        button->setFlat(true);
        button->setStyleSheet("padding: 6px");
        button->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
        button->setToolTip(depend.name());
        if (depend.description().isEmpty())
            button->setText(depend.name() + depend.mod() + depend.version());
        else
            button->setText(depend.name() + depend.mod() + depend.version() + ": " + depend.description());
        m_depsButtonGroup->addButton(button);
        packagesLayout->addWidget(button);
    }
    packagesLabel->show();
}

void MainWindow::loadAppSettings()
{
    const AppSettings settings;

    // Tray icon
    setTrayStatus(m_trayStatus);
#ifndef KDE
    m_trayIcon->show();
#endif

    // Set autosync databases timer
    m_autosyncTimer->loadSettings();

    // Connection
    QNetworkProxy proxy;
    proxy.setType(settings.proxyType());
    if (proxy.type() == QNetworkProxy::HttpProxy || proxy.type() == QNetworkProxy::Socks5Proxy) {
        proxy.setHostName(settings.proxyHost());
        proxy.setPort(settings.proxyPort());
        if (settings.isProxyAuthEnabled()) {
            proxy.setUser(settings.proxyUsername());
            proxy.setPassword(settings.proxyPassword());
        }
    }
    QNetworkProxy::setApplicationProxy(proxy);
}

void MainWindow::loadMainWindowSettings()
{
    const AppSettings settings;
    restoreGeometry(settings.mainWindowGeometry());
    ui->noConfirmAction->setChecked(settings.isNoConfirm());
    m_afterCompletionGroup->actions().at(settings.afterCompletion())->setChecked(true);
}
