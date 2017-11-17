#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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

private:
    QList<UnrealInstall> GetEngineInstalls();

    void RefreshPlugins(UnrealInstall UnrealInstallation);

private:
    Ui::MainWindow *ui;

    UnrealPlugin selectedPlugin;
    UnrealInstall SelectedUnrealInstallation;

    QList<UnrealInstall> UnrealInstallations;

    QList<UnrealPlugin> InstalledPlugins;
};

#endif // MAINWINDOW_H
