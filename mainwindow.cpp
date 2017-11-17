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
        //QVariant((void*)&EngineInstall)
        //qVariantFromValue((void*)&EngineInstall);
        //ui->EngineVersionSelector->addItem(EngineInstall.GetName(), qVariantFromValue((void*)new UnrealInstall(EngineInstall)));
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
    //QStandardPaths::DataLocation
    // QStandardPaths::GenericDataLocation

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
