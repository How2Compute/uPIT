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

private slots:
    void on_EngineVersionSelector_currentIndexChanged(const QString &arg1);

    void on_EngineVersionSelector_currentIndexChanged(int index);

private:
    void SetPlugin(UnrealPlugin Plugin);

    QList<UnrealInstall> GetEngineInstalls();

private:
    Ui::MainWindow *ui;

    UnrealPlugin selectedPlugin;

    QList<UnrealInstall> UnrealInstallations;

    QList<UnrealPlugin> InstalledPlugins;
};

#endif // MAINWINDOW_H
