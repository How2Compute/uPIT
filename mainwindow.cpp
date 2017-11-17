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
#include "ui_mainwindow.h"
#include "pluginselectionbutton.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

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
        ui->InstallPluginButton->setEnabled(false);

        ui->PluginName->setText("");
        ui->PluginDescription->setText("Please Select A Plugin First");
    }
    else
    {
        // Allow the user to install or uninstall the plugin
        if (Plugin.GetInstalled())
        {
            ui->RemovePluginButton->setEnabled(true);
            ui->InstallPluginButton->setEnabled(false);
        }
        else
        {
            ui->InstallPluginButton->setEnabled(true);
            ui->RemovePluginButton->setEnabled(false);
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
        // TODO skip

    }
    QTextStream LauncherInstalledTS(&LauncherInstalled);
    QString LauncherInstalledText = LauncherInstalledTS.readAll();

    QJsonObject jLauncherInstalled = QJsonDocument::fromJson(LauncherInstalledText.toUtf8()).object();

    if (!jLauncherInstalled.contains("InstallationList") && jLauncherInstalled["InstallationList"].isArray())
    {
#ifdef QT_DEBUG
         qDebug() << "Invalid LauncherInstalled.dat Engine Configuration File....Skipping Automatic Engine Detection";
#endif
         // TODO
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

        // TODO these appear to only be launcher versions!
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

    return UnrealInstalls;
}

void MainWindow::on_EngineVersionSelector_currentIndexChanged(int index)
{
    // Use the list's index we got when we where adding these to get the right engine version
    UnrealInstall UnrealInstallation = UnrealInstallations[ui->EngineVersionSelector->itemData(index).toInt()];

    SelectedUnrealInstallation = UnrealInstallation;

#ifdef QT_DEBUG
    qDebug() << "Unreal Version Switched To {Name=" << UnrealInstallation.GetName() << ";Path=" << UnrealInstallation.GetPath() << "}";
#endif

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

    for (UnrealPlugin Plugin : InstalledPlugins)
    {
        // TODO Make the horizontal box scrollable and make the layout neater (eg. not spaced out over the entire thing, but close to eachother vertically)
        ui->PluginList->addWidget(new PluginSelectionButton(Plugin, this));
    }
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
    // TODO - Call a function that will clear & recreate the installed plugins list.

    // Mark this plugin as installed, and then reset it so the installed button will be greyed out & the uninstall will be active.
    selectedPlugin.SetInstalled(true);
    SetPlugin(selectedPlugin);

    // Show the user a popup telling them the plugin successfully installed.
    QMessageBox FinishedPrompt;
    FinishedPrompt.setWindowTitle("Done!");
    FinishedPrompt.setText("We Successfully Installed " + selectedPlugin.GetName() + " to " + SelectedUnrealInstallation.GetName() + "!");
    FinishedPrompt.exec();
}
