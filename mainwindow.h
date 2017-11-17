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

private:
    QList<UnrealInstall> GetEngineInstalls();

private:
    Ui::MainWindow *ui;

    UnrealPlugin selectedPlugin;

    QList<UnrealInstall> UnrealInstallations;

    QList<UnrealPlugin> InstalledPlugins;
};

#endif // MAINWINDOW_H
