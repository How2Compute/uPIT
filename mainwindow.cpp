#include "mainwindow.h"
#include <QStandardPaths>
#include <QtDebug>
#include <QDirIterator>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>
#include <QComboBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QInputDialog>
#include <QSettings>
#include <QListWidget>
#include "ui_mainwindow.h"
#include "pluginlistitem.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Connect the menu's install plugin action up to the install plugin button's handler/slot.
    QObject::connect(ui->actionInstall_Plugin, SIGNAL(triggered()), this, SLOT(on_InstallPluginButton_clicked()));

    // Connect the menu's remove plugin action up to the remove plugin button's handler/slot.
    QObject::connect(ui->actionRemove_Plugin, SIGNAL(triggered()), this, SLOT(on_RemovePluginButton_clicked()));

    // Connect the menu's open plugin action up to the open plugin button's handler/slot.
    QObject::connect(ui->actionOpen_Plugin, SIGNAL(triggered()), this, SLOT(on_OpenPluginButton_clicked()));

    // Clear the plugin details/etc.
    SetPlugin(UnrealPlugin::NoPlugin);

    UnrealInstallations = GetEngineInstalls();

    int i = 0;
    for (UnrealInstall EngineInstall : UnrealInstallations)
    {
        ui->EngineVersionSelector->addItem(EngineInstall.GetName(), QVariant(i));

        i++;
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SetPlugin(UnrealPlugin Plugin)
{
    if (Plugin == UnrealPlugin::NoPlugin)
    {
        // Disable the plugin tools buttons
        ui->RemovePluginButton->setEnabled(false);
        ui->actionRemove_Plugin->setEnabled(false);
        ui->InstallPluginButton->setEnabled(false);
        ui->actionInstall_Plugin->setEnabled(false);

        ui->PluginName->setText("");
        ui->PluginDescription->setText("Please Select A Plugin First");
    }
    else
    {
        // Allow the user to install or uninstall the plugin
        if (Plugin.GetInstalled())
        {
            // Enable the uninstall/remove actions/buttons.
            ui->RemovePluginButton->setEnabled(true);
            ui->actionRemove_Plugin->setEnabled(true);

            // Disable the install plugin actions/buttons.
            ui->InstallPluginButton->setEnabled(false);
            ui->actionInstall_Plugin->setEnabled(false);
        }
        else
        {
            // Enable the install plugin actions/buttons
            ui->InstallPluginButton->setEnabled(true);
            ui->actionInstall_Plugin->setEnabled(true);

            // Disable the uninstall/remove actions/buttons.
            ui->RemovePluginButton->setEnabled(false);
            ui->actionRemove_Plugin->setEnabled(false);
        }

        ui->PluginName->setText(Plugin.GetName());
        ui->PluginDescription->setText(Plugin.GetDescription());
    }

    selectedPlugin = Plugin;
}

QList<UnrealInstall> MainWindow::GetEngineInstalls()
{
    // Get the ue4 versions
    QList<UnrealInstall> UnrealInstalls;

    // Fetch the binary install locations (for windows).

#ifdef Q_OS_WIN

    QStringList paths = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    QString ProgramDataPath;

    for (QString path : paths)
    {
        // Assuming that the *real* one is the only one ending in ProgramData
        if (path.endsWith("ProgramData"))
        {
            ProgramDataPath = path;
            break;
        }

#ifdef QT_DEBUG
    qDebug() << "Found GenericDataLocation Path: " << path;
#endif

    }
    QString LauncherInstalledPath = ProgramDataPath + "/Epic/UnrealEngineLauncher/LauncherInstalled.dat";

    QFile LauncherInstalled(LauncherInstalledPath);

    if (!LauncherInstalled.open(QFile::ReadOnly | QFile::Text))
    {
#ifdef QT_DEBUG
        qDebug() << "Unable To Open LauncherInstalled.dat Engine Configuration File....Skipping Automatic Engine Detection. Path: " << LauncherInstalledPath;
#endif

        return UnrealInstalls;

    }
    QTextStream LauncherInstalledTS(&LauncherInstalled);
    QString LauncherInstalledText = LauncherInstalledTS.readAll();

    QJsonObject jLauncherInstalled = QJsonDocument::fromJson(LauncherInstalledText.toUtf8()).object();

    if (!jLauncherInstalled.contains("InstallationList") && jLauncherInstalled["InstallationList"].isArray())
    {
#ifdef QT_DEBUG
         qDebug() << "Invalid LauncherInstalled.dat Engine Configuration File....Skipping Automatic Engine Detection";
#endif

         return UnrealInstalls;
    }

    for (QJsonValue EngineInstallVal : jLauncherInstalled["InstallationList"].toArray())
    {
        if (!EngineInstallVal.isObject())
        {
#ifdef QT_DEBUG
         qDebug() << "Invalid Launcher Install....skipping this one!";
         continue;
#endif
        }

        QJsonObject EngineInstall = EngineInstallVal.toObject();

        QString EngineLocation = EngineInstall["InstallLocation"].toString();
        QString EngineName = EngineInstall["AppName"].toString();

        // Only get the ue4 builds - filter out the plugins  (that'll be formatted like ConfigBPPlugin_4.17
        if (EngineName.startsWith("UE_"))
        {
            // Add this engine version to the unreal engine installs list
            UnrealInstalls.append(UnrealInstall(EngineName, EngineLocation));

#ifdef QT_DEBUG
            qDebug() << "Found Engine Version: {Name=" << EngineName << ";Locaton=" << EngineLocation << "}";
#endif
        }

    }

#endif

    // Fetch any custom ue4 install paths the user may have added

    // Open up uPIT's settings file
    QSettings Settings("HowToCompute", "uPIT");

    // Create a quick list to add the custom installs to (so they can be added seperate of the *proper* list in case anything goes wrong)
    QList<UnrealInstall> CustomInstalls;

    // Read the Custom Unreal Engine Installs array from the settings object.
    int size = Settings.beginReadArray("CustomUnrealEngineInstalls");
    for (int i = 0; i < size; ++i) {
        // Get this element out of the settings
        Settings.setArrayIndex(i);

        // Extract the installation's name & path
        QString InstallName = Settings.value("Name").toString();
        QString InstallPath = Settings.value("Path").toString();

        // Create an UnrealInstall based on the name & path, and add it to the Custom Installs list.
        CustomInstalls.append(UnrealInstall(InstallName, InstallPath));
    }

    // Done reading, so "close" the array.
    Settings.endArray();

    // Add the list of custom installs to the list of (binary) Unreal Engine installations.
    UnrealInstalls += CustomInstalls;

    // Return the final list of UE4 binary installs & custom installs.
    return UnrealInstalls;
}

void MainWindow::RefreshPlugins(UnrealInstall UnrealInstallation)
{
    QDir MarketplaceDir(UnrealInstallation.GetPath() + "/Engine/Plugins/Marketplace");

    // Where we will install plugins to by default
    QDir CustomDir(UnrealInstallation.GetPath() + "/Engine/Plugins/uPIT");

    QList<QPair<QString, PluginSource>> PluginPaths;

    if (MarketplaceDir.exists())
    {
        QDirIterator marketplaceIt(MarketplaceDir.path(), QStringList() << "*.uplugin", QDir::Files, QDirIterator::Subdirectories);

        while (marketplaceIt.hasNext())
        {
            QString PluginPath = marketplaceIt.next();
            PluginPaths.append(QPair<QString, PluginSource>(PluginPath,PluginSource::PS_MARKETPLACE));

    #ifdef QT_DEBUG
            qDebug() << "Marketplace Plugin Detected: " << PluginPath;
    #endif
        }
    }

    if (CustomDir.exists())
    {
        QDirIterator CustomIt(CustomDir.path(), QStringList() << "*.uplugin", QDir::Files, QDirIterator::Subdirectories);

        while (CustomIt.hasNext())
        {
            QString PluginPath = CustomIt.next();
            PluginPaths.append(QPair<QString, PluginSource>(PluginPath,PluginSource::PS_uPIT));

    #ifdef QT_DEBUG
            qDebug() << "\"Custom\" Plugin Detected: " << PluginPath;
    #endif
        }
    }

    QList<UnrealPlugin> Plugins;

    for (QPair<QString, PluginSource> Plugin : PluginPaths)
    {
        QFile PluginMeta(Plugin.first);

        if (!PluginMeta.open(QFile::ReadOnly | QFile::Text))
        {
    #ifdef QT_DEBUG
            qDebug() << "Unable To Open UPlugin File For Plugin Located At: " << Plugin.first << "...Skipping Plugin!";
    #endif
            continue;
        }

        QTextStream PluginMetaTS(&PluginMeta);
        QString PluginMetaString = PluginMetaTS.readAll();

        QJsonObject jPlugin = QJsonDocument::fromJson(PluginMetaString.toUtf8()).object();

        QString PluginName = jPlugin["FriendlyName"].toString();
        QString PluginDescription = jPlugin["Description"].toString();

        // TODO more metadata like the author, possibly the docs, etc.

        Plugins.append(UnrealPlugin(PluginName, Plugin.first, PluginDescription, UnrealInstallation.GetName(), Plugin.second, true));

#ifdef QT_DEBUG
        qDebug() << "Added Plugin With Friendly Name: " << PluginName;
#endif
    }

    InstalledPlugins = Plugins;
    if (InstalledPlugins.length() > 0)
    {
        SetPlugin(InstalledPlugins[0]);
    }

    // Clear the items/plugins in the installed plugins list so we can replace them with the new list.
    // TODO Does this only clear the ones in view? (eg. if there where like 100 plugins and not all where in view, does it still clear them all?)
    ui->PluginList->clear();

    // Add the new plugins to the list
    for (UnrealPlugin Plugin : InstalledPlugins)
    {   // Add this plugin as the next (by using ->count() to get the next unused index) "slot" in the list.
        // For some wierd reason the text needs to be set for a second time when using insertItem over giving it the parent directory (which causes other issues).
        PluginListItem *PluginEntry = new PluginListItem(Plugin, ui->PluginList);
        PluginEntry->setText(Plugin.GetName());
        ui->PluginList->insertItem(ui->PluginList->count(), PluginEntry);
    }
}

void MainWindow::on_EngineVersionSelector_currentIndexChanged(int index)
{
    // Use the list's index we got when we where adding these to get the right engine version
    UnrealInstall UnrealInstallation = UnrealInstallations[ui->EngineVersionSelector->itemData(index).toInt()];

    SelectedUnrealInstallation = UnrealInstallation;

#ifdef QT_DEBUG
    qDebug() << "Unreal Version Switched To {Name=" << UnrealInstallation.GetName() << ";Path=" << UnrealInstallation.GetPath() << "}";
#endif

    // Refresh the plugin list for this new engine version
    RefreshPlugins(SelectedUnrealInstallation);
}

void MainWindow::on_OpenPluginButton_clicked()
{
    // Allow the user to browse to where the uplugin file is located
    QString UpluginFilePath = QFileDialog::getOpenFileName(this, "Open Your Plugin's .uplugin File", "", "Unreal Plugin (*.uplugin)");

    // If the string was blank, prompt the user. - TODO evaluate whether or not this is even needed.
    if (UpluginFilePath.isEmpty())
    {
        // It appears no UPLUGIN file was selected, show a popup error
#ifdef QT_DEBUG
        qDebug() << "No Or Invalid .uplugin File Selected";
#endif
        QMessageBox ErrorPopup;
        ErrorPopup.setWindowTitle("Invalid Unreal Plugin Selected");
        ErrorPopup.setText("You A. didn't select a uplugin file, or B. the file was invalid. Please try this again.");
        ErrorPopup.exec();
        return;
    }

    // Parse the UPlugin file & create an UnrealPlugin based on this (TODO abstract this functionality for reusability)
    QFile PluginMeta(UpluginFilePath);

    // Attempt to open the plugin's .uplugin file and pop up an error is there where any issues.
    if (!PluginMeta.open(QFile::ReadOnly | QFile::Text))
    {
#ifdef QT_DEBUG
        qDebug() << "Unable To Open UPlugin File For Plugin Located At: " << UpluginFilePath << "...Skipping Plugin!";
#endif
        QMessageBox ErrorPopup;
        ErrorPopup.setWindowTitle("Read Error");
        ErrorPopup.setText("Unable To Read The Selected UPLUGIN File.");
        ErrorPopup.exec();
        return;
    }

    QTextStream PluginMetaTS(&PluginMeta);
    QString PluginMetaString = PluginMetaTS.readAll();

    QJsonObject jPlugin = QJsonDocument::fromJson(PluginMetaString.toUtf8()).object();

    QString PluginName = jPlugin["FriendlyName"].toString();
    QString PluginDescription = jPlugin["Description"].toString();

    // Set the newly opened plugin so the user can install it.
    SetPlugin(UnrealPlugin(PluginName, UpluginFilePath, PluginDescription, SelectedUnrealInstallation.GetName(), PluginSource::PS_uPIT, false));

}

void MainWindow::on_InstallPluginButton_clicked()
{
    // Prompt the user to double check they indeed want to install this plugin
    QMessageBox UserConsentPrompt;
    UserConsentPrompt.setWindowTitle("Are You Sure?");
    UserConsentPrompt.setText("Are You Sure You Want To Install: " + selectedPlugin.GetName() + "?");
    UserConsentPrompt.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

    int UserResponse = UserConsentPrompt.exec();
    bool bShouldInstall;

    // Which button did the user press?
    switch (UserResponse)
    {
        case QMessageBox::Ok:
            bShouldInstall = true;
            break;
        case QMessageBox::Cancel:
            bShouldInstall = false;
            break;
        default:
            // Invalid Response Received - Don't Care About It Too Much
            bShouldInstall = false;
#ifdef QT_DEBUG
            qDebug() << "Invalid User Conscent Prompt Received";
#endif
            break;
    }

    // Exit out if we don't need to install.
    if (!bShouldInstall)
    {
        return;
    }

    // Check whether or not there is a Binaries/<Platform> file
    QString PluginBasePath = QFileInfo(selectedPlugin.GetPath()).absoluteDir().absolutePath();

    QDir PluginBinariesPath;

#ifdef Q_OS_WIN
    // Get the Win64 binaries folder
    PluginBinariesPath.setPath(PluginBasePath + "/Binaries/Win64");
#elif defined Q_OS_WIN32
    // Get the Win32 binaries folder
     PluginBinariesPath.setPath(PluginBasePath + "/Binaries/Win32");    // TODO No plugins to test this with :(
#elif defined Q_OS_LINUX
    // Get the Linux binaries folder
    PluginBinariesPath.setPath(PluginBasePath + "/Binaries/Linux");
#else
    // Unsupported OS! Won't care about binaries (so assume they simply exist).
    PluginBinariesPath.setPath(PluginBasePath + "/Binaries");
#endif

#ifdef QT_DEBUG
    qDebug() << "Plugin Base Path: " << PluginBasePath << "Binaries Path: " << PluginBinariesPath;
#endif

    // Does the binaries path exist, and does it contain any files? (first condition there to allow for unsupported platforms)
    if (PluginBinariesPath.path() != PluginBasePath + "/Binaries" && !(PluginBinariesPath.exists() && !PluginBinariesPath.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden).isEmpty()))
    {
        // If not, ask the user whether or not they want to rebuild it
        QMessageBox AskForRebuildPrompt;
        AskForRebuildPrompt.setWindowTitle("Rebuild Plugin Binaires?");
        AskForRebuildPrompt.setText("We Where Unable To Detect Binaries For You Platform - Would You Like To (Re-)Build These?");
        AskForRebuildPrompt.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

        int UserResponse = AskForRebuildPrompt.exec();
        bool bShouldRebuild;

        // Which button did the user press?
        switch (UserResponse)
        {
            case QMessageBox::Yes:
                bShouldRebuild = true;
                break;
            case QMessageBox::No:
                bShouldRebuild = false;
                break;
            default:
                // Invalid Response Received - Don't Care About It Too Much
                bShouldRebuild = false;
    #ifdef QT_DEBUG
                qDebug() << "Invalid User Conscent Prompt Received";
    #endif
                break;
        }

        // If they click yes to this, run the Rocket command based on the currently selected engine & build the plugins into a temporary appdata folder
        if (bShouldRebuild)
        {
            bool bRunningWindows;

#ifdef Q_OS_WIN
                bRunningWindows = true;
#elif defined Q_OS_WIN32
                bRunningWindows = true;
#else
                // TODO Add Linux Support Too!
                bRunningWindows = false;
#endif
                if (bRunningWindows)
                {
                    QString RunUATPath = SelectedUnrealInstallation.GetPath() + "/Engine/Build/BatchFiles/RunUAT.bat";
                    QStringList RunUATFlags;
                    RunUATFlags << "BuildPlugin";
                    RunUATFlags << "-Plugin=" + selectedPlugin.GetPath();

                    // Get or create a path where to package the plugin
                    QDir PackageLocation = QStandardPaths::standardLocations(QStandardPaths::DataLocation)[0] + "/BuiltApps/" + selectedPlugin.GetName() + "-" + SelectedUnrealInstallation.GetName();

                    if (!PackageLocation.exists())
                    {
                        // Create the directory (use a default constructor due to the way mdkir works)
                        QDir().mkdir(PackageLocation.path());
                    }

                    RunUATFlags << "-Package=" + PackageLocation.path();
                    RunUATFlags << "-Rocket";

#ifdef QT_DEBUG
                    qDebug() << "Going to run " << RunUATPath << " with the flags: " << RunUATFlags << " to build this plugin...";
#endif

                    // Run the UAT and wait until it's finished - TODO show the user some feedback so they don't think the app is hanging
                    QProcess p;
                    p.start(RunUATPath, RunUATFlags);

                    // Give it no timeout because plugins can be pretty large, and builds, depending on the system, can take (freaking) forever!
                    p.waitForFinished(-1);

                    if (p.exitStatus() == QProcess::NormalExit && p.exitCode() == 0)
                    {
#ifdef QT_DEBUG
                        qDebug() << "Successfully Built Plugin Binaries.";
#endif
                        // Since we successfully built the plugin, set the plugin's path to this new location:
                        /// It should be safe to assume the host project only has one project plugin
                        QDirIterator PackageLocationIt(PackageLocation.path() + "/HostProject/Plugins",  QStringList() << "*.uplugin", QDir::Files, QDirIterator::Subdirectories);
                        // TODO Does the plugin remain in HostProject? I believe it doesn't, but can't check as of writing this line.

                        while (PackageLocationIt.hasNext())
                        {
                            QString PluginPath = PackageLocationIt.next();
                            PluginBasePath = QFileInfo(PluginPath).absoluteDir().absolutePath();
                        }
                    }
                    else
                    {
#ifdef QT_DEBUG
                        qDebug() << "Finished Building Plugin Binaries, But Failed. Output Log:";
                        qDebug() <<  QString(p.readAll());
#endif

                        // If not, ask the user whether or not they want to rebuild it
                        QMessageBox BuildFailedPrompt;
                        BuildFailedPrompt.setWindowTitle("Failed To Build Plugin Binaries");
                        BuildFailedPrompt.setText("There Was An Issue Building Your Plugin's Binaries. Would You Still Like To Continue On With The Installation?");
                        BuildFailedPrompt.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

                        int UserResponse = BuildFailedPrompt.exec();

                        // Which button did the user press?
                        switch (UserResponse)
                        {
                            case QMessageBox::Ok:
                                break;
                            case QMessageBox::Cancel:
                                return;
                                break;
                        }
                    }

                }
        }
    }

    // Copy over all of the plugin's files to the engine's plugins directory (use the uPIT subdirectory to keep a tab on them)
    QFileInfoList Files = QDir(PluginBasePath).entryInfoList(QDir::AllEntries | QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Hidden);

#ifdef QT_DEBUG
        qDebug() << "Copying Plugin Files...";
#endif

    QString DestinationDirectory = SelectedUnrealInstallation.GetPath() + "/Engine/Plugins/uPIT";

    // Create the uPIT directory if it doesn't yet exist.
    if (!QDir(DestinationDirectory).exists())
    {
        QDir().mkdir(DestinationDirectory);
    }

    // Create the plugin directory if it doesn't yet exist.
    if (!QDir(DestinationDirectory + "/" + QDir(PluginBasePath).dirName()).exists())
    {
        QDir().mkdir(DestinationDirectory + "/" + QDir(PluginBasePath).dirName());
    }

    // TODO Make a progress bar? (that could also be used with like 0% then 50% then 100% when building binaries as it only has a few things we can track)

    QDirIterator it(PluginBasePath, QStringList() << "*.*", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        // Get The Next File We Need To Copy
        QString FilePath = it.next();

        // Fetch/Create The Source And Destination Paths
        QString Source = FilePath;
        QString Destination = DestinationDirectory + "/" + QDir(PluginBasePath).dirName() + FilePath.remove(PluginBasePath);

        // Create the directory if it doesn't yet exist.
        if (!QDir(QFileInfo(Destination).absoluteDir().absolutePath()).exists())
        {
            QDir().mkdir(QFileInfo(Destination).absoluteDir().absolutePath());
        }

        QFile(Source).copy(Destination);

#ifdef QT_DEBUG
        qDebug() << Source << " -> " << Destination;
#endif
    }

#ifdef QT_DEBUG
        qDebug() << "Finished Copying Files.";
#endif

    // Tell the user we're done, and be sure to update the plugin so it's added to the engine & set to installed (so the user can only press the remove button on the plugin).
    RefreshPlugins(SelectedUnrealInstallation);

    // Mark this plugin as installed, and then reset it so the installed button will be greyed out & the uninstall will be active.
    selectedPlugin.SetInstalled(true);
    SetPlugin(selectedPlugin);

    // Show the user a popup telling them the plugin successfully installed.
    QMessageBox FinishedPrompt;
    FinishedPrompt.setWindowTitle("Done!");
    FinishedPrompt.setText("We Successfully Installed " + selectedPlugin.GetName() + " to " + SelectedUnrealInstallation.GetName() + "!");
    FinishedPrompt.exec();
}

void MainWindow::on_RemovePluginButton_clicked()
{
    // Double check with the user that they indeed want to uninstall the plugin.
    QMessageBox UserConsentPrompt;
    UserConsentPrompt.setWindowTitle("Are You Sure?");
    UserConsentPrompt.setText("Are You Sure You Want To Uninstall " + selectedPlugin.GetName() + " from " + SelectedUnrealInstallation.GetName() + "?");
    UserConsentPrompt.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

    int UserResponse = UserConsentPrompt.exec();
    bool bShouldUninstall;

    // Which button did the user press?
    switch (UserResponse)
    {
        case QMessageBox::Ok:
            bShouldUninstall = true;
            break;
        case QMessageBox::Cancel:
            bShouldUninstall = false;
            break;
        default:
            // Invalid Response Received - Don't Care About It Too Much
            bShouldUninstall = false;
#ifdef QT_DEBUG
            qDebug() << "Invalid User Conscent Prompt Response";
#endif
            break;
    }

    // If the user didn't intend to uninstall, exit out now so the uninstall logic isn't run.
    if (!bShouldUninstall)
    {
        return;
    }

    // Store the plugin's name so we can use it when we give the success prompt (as selectedPlugin will be set to blank when it's finished uninstalling).
    QString PluginName = selectedPlugin.GetName();

    // Remove the plugin's files.
    QDir(QFileInfo(selectedPlugin.GetPath()).absoluteDir().absolutePath()).removeRecursively();

    // Set the selected plugin to the blank plugin/NoPlugin.
    SetPlugin(UnrealPlugin::NoPlugin);

    // Refresh the installed plugins list.
    RefreshPlugins(SelectedUnrealInstallation);

    // Tell the user we successfully uninstalled the plugin.
    QMessageBox SuccessPrompt;
    SuccessPrompt.setWindowTitle("Done!");
    SuccessPrompt.setText("Successfully Uninstalled " + PluginName + " from " + SelectedUnrealInstallation.GetName() + "!");
    SuccessPrompt.exec();
}

void MainWindow::on_actionAdd_Unreal_Engine_Install_triggered()
{
    // NOTE: Assume users will go into the root (eg .../UE_4.17/ and not .../UE_4.17/Engine/ or something like that)

    QString InstallDirectory = QFileDialog::getExistingDirectory(this, "Open The Engine Install", "");

#ifdef QT_DEBUG
    qDebug() << "User Selected Installation Directory: " << InstallDirectory;
#endif

    if (InstallDirectory.isEmpty())
    {
        // The install directory was blank, meaning that the user (probably) canceled the dialog. Don't continue on, but don't show an error either.
        return;
    }
    else if (!QDir(InstallDirectory).exists())
    {
#ifdef QT_DEBUG
        qDebug() << "Selected Install Directory Does Not Exist!";
#endif
        // This literally shouldn't ever happen, so just drop the request.
        return;
    }

    bool bGotName;
    QString EngineName = QInputDialog::getText(this, "Add Custom UE4 Installation", "Please Give This Installation A (Descriptive) Name.", QLineEdit::Normal, "CUSTOM_UNREAL_INSTALL", &bGotName);

    if (!bGotName)
    {
        // The user pressed the cancel button, so don't actually add the engine (assume they don't want to add the engine anymore).
        return;
    }

    if (EngineName.isEmpty())
    {
        // The user entered a blank name, which isn't allowed, so don't add the engine and prompt them to retry.
        QMessageBox ErrorPrompt;
        ErrorPrompt.setWindowTitle("Uh Oh!");
        ErrorPrompt.setText("The Name You Entered Was Unfortionately Invalid (Blank Name). Please Try Again.");
        ErrorPrompt.setStandardButtons(QMessageBox::Ok);
        ErrorPrompt.exec();
        return;
    }

    // Add this newly created custom installation to the list of custom ue4 installations.

    QSettings Settings("HowToCompute", "uPIT");

    UnrealInstall CustomInstall(EngineName, InstallDirectory);

    // Get the number of items in the array.
    int size = Settings.beginReadArray("CustomUnrealEngineInstalls");
    Settings.endArray();

    // Append this install to the engine's custom installs list.
    Settings.beginWriteArray("CustomUnrealEngineInstalls");
    Settings.setArrayIndex(size);
    Settings.setValue("Name", CustomInstall.GetName());
    Settings.setValue("Path", CustomInstall.GetPath());
    Settings.endArray();

    // Add the new install to the (known) unreal installs list & add it to the UI dropdown.
    UnrealInstallations.append(CustomInstall);

    ui->EngineVersionSelector->addItem(CustomInstall.GetName(), QVariant(ui->EngineVersionSelector->count()));
}

void MainWindow::on_PluginList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    // Cast the (generic) item into a (usable) plugin item.
    PluginListItem *PluginItem = (PluginListItem*)current;//dynamic_cast<PluginSelectionButton*>(current);

    // Check whether or not the cast was successful.
    if (!PluginItem)
    {
#ifdef QT_DEBUG
        qDebug() << "One Of The (Plugin) List Widget Items Was Invalid!";
#endif
        return;
    }

   // "Select" the item's button
   SetPlugin(PluginItem->GetPlugin());

#ifdef QT_DEBUG
   qDebug() << "Selected Plugin: " << PluginItem->GetPlugin().GetName();
#endif
}
