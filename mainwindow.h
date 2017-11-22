#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QProcess>
#include "unrealplugin.h"
#include "unrealinstall.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void SetPlugin(UnrealPlugin Plugin);

private slots:
    void on_EngineVersionSelector_currentIndexChanged(int index);

    // Called when the user wants to open a non-installed plugin.
    /// Should ask the user to browse to it, create a UnrealPlugin based on this, and then set it using SetPlugin();
    void on_OpenPluginButton_clicked();

    // Called when the user wants to install a plugin
    /// Should ask for a confirmation, check if there are any binaries (I don't believe checking engine version here is possible), if they aren't there ask if they want to rebuild (Using rocket), and finally install the plugin.
    void on_InstallPluginButton_clicked();

    // Called when the user wants to uninstall/remove a plugin.
    /// Should ask for a confirmation, and then remove the files & all records of the plugin currently active.
    void on_RemovePluginButton_clicked();

    void on_actionAdd_Unreal_Engine_Install_triggered();

    void on_PluginList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_PluginBuild_complete(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QList<UnrealInstall> GetEngineInstalls();

    void RefreshPlugins(UnrealInstall UnrealInstallation);

    void CopyPluginFiles(QString SourcePath, QString DestinationPath);

    void PluginInstallComplete();

private:
    Ui::MainWindow *ui;

    UnrealPlugin selectedPlugin;
    UnrealInstall SelectedUnrealInstallation;

    QList<UnrealInstall> UnrealInstallations;

    QList<UnrealPlugin> InstalledPlugins;

    QMap<QString, QString> PluginInstallState;
};

#endif // MAINWINDOW_H
