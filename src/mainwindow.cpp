#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tasksdialog.h"
#include "autosynctimer.h"
#include "systemtray.h"
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
#include <QShortcut>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Show a message that the application is already running
    connect(qobject_cast<SingleApplication*>(SingleApplication::instance()), &SingleApplication::instanceStarted, this, &MainWindow::showAppRunningMessage);

    // Packages view reactions
    connect(ui->packagesView->model(), &PackagesModel::databaseLoadingMessageChanged, this, &MainWindow::setStatusBarMessage);
    connect(ui->packagesView->model(), &PackagesModel::firstPackageAvailable, this, &MainWindow::processFirstPackageAvailable);
    connect(ui->packagesView->model(), &PackagesModel::databaseStatusChanged, this, &MainWindow::processDatabaseStatusChanged);
    connect(ui->packagesView, &PackagesView::operationsCountChanged, this, &MainWindow::processOperationsCountChanged);

    // Shortcuts
    m_changeModeShortcut = new QShortcut(this);
    m_searchPackagesShortcut = new QShortcut(this);
    connect(m_changeModeShortcut, &QShortcut::activated, this, &MainWindow::changeSearchMode);
    connect(m_searchPackagesShortcut, &QShortcut::activated, ui->searchPackagesEdit, qOverload<>(&SearchEdit::setFocus));

    // Terminal
    m_pacman = new Pacman(this);
    m_pacman->setTasks(ui->packagesView);
    connect(m_pacman, &Pacman::finished, this, &MainWindow::processTerminalFinish);
    connect(m_pacman, &Pacman::started, this, &MainWindow::processTerminalStart);

    // Autosync
    m_autosyncTimer = new AutosyncTimer(this);
    connect(m_autosyncTimer, &AutosyncTimer::timeout, m_pacman, &Pacman::syncDatabase); // Automatically sync databases in background

    // Select package when clicking on dependencies
    m_depsButtonGroup = new QButtonGroup(this);
    connect(m_depsButtonGroup, qOverload<QAbstractButton *>(&QButtonGroup::buttonClicked), this, &MainWindow::findDepend);

    // Make after completion actions exclusive
    m_afterCompletionGroup = new QActionGroup(this);
    m_afterCompletionGroup->addAction(ui->waitForInputAction);
    m_afterCompletionGroup->addAction(ui->shutdownAction);
    m_afterCompletionGroup->addAction(ui->rebootAction);
    connect(m_afterCompletionGroup, &QActionGroup::triggered, this, &MainWindow::setAfterTasksCompletionAction);

    // System tray context menu
    m_trayMenu = new QMenu(this);
    m_trayMenu->addAction(QIcon::fromTheme("window"), tr("Show window"), this, &MainWindow::show);
    m_trayMenu->addAction(ui->settingsAction->icon(), ui->settingsAction->iconText(), this, &MainWindow::openSettings);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(QIcon::fromTheme("system-upgrade"), tr("Sync and upgrade (-Syu)"), this, &MainWindow::syncAndUpgrade);
    m_trayMenu->addAction(ui->upgradeButton->icon(), ui->upgradeButton->text(), this, &MainWindow::upgrade);
    m_trayMenu->addAction(ui->syncButton->icon(), ui->syncButton->text(), this, &MainWindow::syncRepositories);
    m_trayMenu->addAction(ui->reloadButton->icon(), ui->reloadButton->text(), this, &MainWindow::reloadDatabase);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(QIcon::fromTheme("application-exit"), tr("Exit"), SingleApplication::instance(), &SingleApplication::quit);

    // System tray
    m_trayIcon = new SystemTray(this);
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
    settings.setMainWindowGeometry(saveGeometry());

    settings.setInstantSearchEnabled(ui->instantSearchAction->isChecked());
    settings.setNoConfirm(ui->noConfirmAction->isChecked());
    settings.setForce(ui->forceAction->isChecked());

    QAction *afterCompletionAction = m_afterCompletionGroup->checkedAction();
    const auto afterCompletion = static_cast<Pacman::AfterCompletion>(m_afterCompletionGroup->actions().indexOf(afterCompletionAction));
    settings.setAfterCompletion(afterCompletion);

    delete ui;
}

void MainWindow::installLocalPackage()
{
    QFileDialog dialog(this, tr("Select package"));
    dialog.setNameFilter(tr("Pacman package (*.pkg.tar.xz);;All files(*)"));
    dialog.setDirectory(QDir::homePath());
    dialog.setFileMode(QFileDialog::ExistingFile);

    if (!dialog.exec())
        return;

    m_pacman->installLocalPackage(dialog.selectedFiles().at(0));
}

void MainWindow::installLocalPackageAsDepend()
{
    QFileDialog dialog(this, tr("Select package"));
    dialog.setNameFilter(tr("Pacman package (*.pkg.tar.xz);;All files(*)"));
    dialog.setDirectory(QDir::homePath());
    dialog.setFileMode(QFileDialog::ExistingFile);

    if (!dialog.exec())
        return;

    m_pacman->installLocalPackage(dialog.selectedFiles().at(0), true);
}

void MainWindow::exit()
{
    SingleApplication::exit();
}

void MainWindow::setInstantSearch(bool enabled)
{
    ui->searchPackagesEdit->setInstantSearchEnabled(enabled);
    if (enabled)
        searchPackages(ui->searchPackagesEdit->text());
}

void MainWindow::setNoConfirm(bool enabled)
{
    m_pacman->setNoConfirm(enabled);
}

void MainWindow::setForce(bool enabled)
{
    m_pacman->setForce(enabled);
}

void MainWindow::openHistoryFile()
{
    const PacmanSettings pacmanSettings;
    const QFileInfo logFile = pacmanSettings.logFile();
    QDesktopServices::openUrl(logFile.filePath());
}

void MainWindow::openHistoryFileFolder()
{
    const PacmanSettings pacmanSettings;
    const QFileInfo logFile = pacmanSettings.logFile();
    QDesktopServices::openUrl(logFile.dir().path());
}

void MainWindow::openSettings()
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
    applyTasks();
}

void MainWindow::syncRepositories()
{
    ui->packagesView->setSyncRepositories(true);
    ui->packagesView->setUpgradePackages(false);
    applyTasks();
}

void MainWindow::upgrade()
{
    ui->packagesView->setUpgradePackages(true);
    ui->packagesView->setSyncRepositories(false);
    applyTasks();
}

void MainWindow::applyTasks()
{
    TasksDialog dialog(m_pacman, ui->packagesView, ui->menuBar, this);
    if (dialog.exec())
        m_pacman->executeTasks();

    // Set pacman options back (thay may be changed in dialog)
    setNoConfirm(ui->noConfirmAction->isChecked());
    setForce(ui->forceAction->isChecked());
    setAfterTasksCompletionAction(m_afterCompletionGroup->checkedAction());
}

void MainWindow::setSyncDatabase(bool sync)
{
    ui->packagesView->setSyncRepositories(sync);

    if (sync) {
        ui->upgradeButton->setEnabled(true); // Enable upgrade button even if no updates available
    } else if (ui->packagesView->model()->outdatedPackages().isEmpty()) {
        ui->upgradeButton->setChecked(false);
        ui->upgradeButton->setEnabled(false);
    }
}

void MainWindow::setUpgradeDatabase(bool upgrade)
{
    ui->packagesView->setUpgradePackages(upgrade);
}

void MainWindow::reloadDatabase()
{
    ui->packagesView->model()->reloadRepoPackages();
}

void MainWindow::openInBrowser()
{
    QUrl url;
    const Package *package = ui->packagesView->currentPackage();
    if (package->repo() == "aur")
        url = "https://aur.archlinux.org/packages/" + package->name();
    else
        url = "https://www.archlinux.org/packages/" + package->repo() + '/' + package->arch() + '/' + package->name();

    QDesktopServices::openUrl(url);
}

void MainWindow::setSearchMode(int mode)
{
    // Disable search by description for AUR
    if (mode == PackagesModel::AUR) {
        qobject_cast<QStandardItemModel *>(ui->searchByComboBox->model())->item(3)->setEnabled(false);
        if (ui->searchByComboBox->currentIndex() == 3)
            ui->searchByComboBox->setCurrentIndex(0);
    } else {
        qobject_cast<QStandardItemModel *>(ui->searchByComboBox->model())->item(3)->setEnabled(true);
    }

    ui->packagesView->model()->setMode(static_cast<PackagesModel::Mode>(mode));
    searchPackages(ui->searchPackagesEdit->text()); // Search packages
}

void MainWindow::searchPackages(const QString &text)
{
    const auto filterType = static_cast<PackagesView::SearchType>(ui->searchByComboBox->currentIndex());
    ui->packagesView->search(text, filterType);
}

void MainWindow::showAppRunningMessage()
{
    auto *message = new QMessageBox(QMessageBox::Information, SingleApplication::applicationName(), tr("The application is already running"));
    message->setAttribute(Qt::WA_DeleteOnClose); // Need to allocate on heap to avoid crash!
    m_trayIcon->showMainWindow();
    message->show();
}

void MainWindow::displayPackage(Package *package)
{
    // Reset loaded tabs information
    m_packageInfoLoaded = false;
    m_packageDepsLoaded = false;
    m_packageFilesLoaded = false;

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

void MainWindow::setPackageTab(int index)
{
    const Package *package = ui->packagesView->currentPackage();

    switch (index) {
    case 0:
        if (!m_packageInfoLoaded)
            loadPackageInfo(package);
        return;
    case 1:
        if (!m_packageDepsLoaded)
            loadPackageDeps(package);
        return;
    case 2:
        if (!m_packageFilesLoaded)
            loadPackageFiles(package);
        return;
    default:
        return;
    }
}

void MainWindow::setStatusBarMessage(const QString &text)
{
    statusBar()->showMessage(text);
}

void MainWindow::changeSearchMode()
{
    switch (ui->searchModeComboBox->currentIndex()) {
    case PackagesModel::Repo:
        ui->searchModeComboBox->setCurrentIndex(PackagesModel::AUR);
        break;
    case PackagesModel::AUR:
        ui->searchModeComboBox->setCurrentIndex(PackagesModel::Repo);
        break;
    }
}

void MainWindow::findDepend(QAbstractButton *button)
{
    // Clear filter
    ui->searchPackagesEdit->clear();
    if (ui->searchModeComboBox->currentIndex() != PackagesModel::Repo)
        ui->searchModeComboBox->setCurrentIndex(PackagesModel::Repo);
    else if (!ui->instantSearchAction->isChecked())
        searchPackages(ui->searchPackagesEdit->text());

    // Search package in repo first
    const bool found = ui->packagesView->find(button->toolTip());
    if (!found) {
        // Search in AUR
        ui->searchPackagesEdit->setText(button->toolTip());
        ui->searchModeComboBox->setCurrentIndex(PackagesModel::AUR);
    }
}

void MainWindow::processDatabaseStatusChanged(PackagesModel::DatabaseStatus status)
{
    m_trayIcon->setTrayStatus(status, ui->packagesView->model()->outdatedPackages().size());

    switch (status) {
    case PackagesModel::Loading:
        ui->reloadButton->setEnabled(false);
        ui->syncButton->setEnabled(false);

        // Also uncheck buttons
        ui->syncButton->setChecked(false);
        ui->upgradeButton->setChecked(false);
        break;
    case PackagesModel::NoUpdates:
        ui->reloadButton->setEnabled(true);
        ui->syncButton->setEnabled(true);
        ui->upgradeButton->setEnabled(false);

        break;
    case PackagesModel::UpdatesAvailable:
        ui->reloadButton->setEnabled(true);
        ui->syncButton->setEnabled(true);
        ui->upgradeButton->setEnabled(true);
        break;
    }
}

void MainWindow::processFirstPackageAvailable()
{
    ui->packagesView->setCurrentIndex(ui->packagesView->model()->index(0, 0));
}

void MainWindow::processOperationsCountChanged(int tasksCount)
{
    if (tasksCount > 0) {
        ui->applyButton->setToolTip("Apply tasks (" + QString::number(tasksCount) + ')');
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
    processDatabaseStatusChanged(PackagesModel::Loading);
}

void MainWindow::processTerminalFinish(int exitCode)
{
    if (exitCode != 0) {
        if (ui->packagesView->model()->outdatedPackages().isEmpty())
            processDatabaseStatusChanged(PackagesModel::NoUpdates);
        else
            processDatabaseStatusChanged(PackagesModel::UpdatesAvailable);
    } else {
        reloadDatabase();
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

    m_packageInfoLoaded = true;
}

void MainWindow::loadPackageDeps(const Package *package)
{
    loadDepsButtons(0, package->provides());
    loadDepsButtons(1, package->replaces());
    loadDepsButtons(2, package->conflicts());
    loadDepsButtons(3, package->depends());
    loadDepsButtons(4, package->optdepends());

    m_packageDepsLoaded = true;
}

void MainWindow::loadPackageFiles(const Package *package)
{
    ui->filesView->model()->setPaths(package->files());
    m_packageFilesLoaded = true;
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
    m_trayIcon->setTrayStatus(ui->packagesView->model()->databaseStatus(), ui->packagesView->model()->outdatedPackages().size());

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

    // Shortcuts
    m_changeModeShortcut->setKey(QKeySequence(settings.changeModeShortcut()));
    m_searchPackagesShortcut->setKey(QKeySequence(settings.searchPackagesShortcut()));
}

void MainWindow::loadMainWindowSettings()
{
    const AppSettings settings;
    restoreGeometry(settings.mainWindowGeometry());
    m_afterCompletionGroup->actions().at(settings.afterCompletion())->setChecked(true);
    ui->noConfirmAction->setChecked(settings.isNoConfirm());
    ui->forceAction->setChecked(settings.isForce());
    ui->instantSearchAction->setChecked(settings.isInstantSearchEnabled());
}
