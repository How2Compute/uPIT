#ifndef PLUGINSELECTIONBUTTON_H
#define PLUGINSELECTIONBUTTON_H

#include <QWidget>
#include "unrealplugin.h"
#include "mainwindow.h"
#include <QPushButton>

class PluginSelectionButton : public QWidget
{
    Q_OBJECT
public:
    explicit PluginSelectionButton(QWidget *parent = nullptr);
    PluginSelectionButton(UnrealPlugin Plugin, MainWindow *window);

signals:

private slots:
    void onPluginSelected(bool checked);

private:
    QPushButton *button;
    MainWindow *mWindow;
    UnrealPlugin Plugin;

};

#endif // PLUGINSELECTIONBUTTON_H
