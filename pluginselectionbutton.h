#ifndef PLUGINSELECTIONBUTTON_H
#define PLUGINSELECTIONBUTTON_H

#include <QWidget>
#include "unrealplugin.h"
#include "mainwindow.h"
#include <QPushButton>
#include <QListWidgetItem>

class PluginSelectionButton : public QListWidgetItem
{
    //Q_OBJECT
public:
    PluginSelectionButton(UnrealPlugin Plugin, QListWidget *parent);

    // Returns the plugin that this item was created with.
    UnrealPlugin GetPlugin();

private:
    QListWidgetItem *button;
    UnrealPlugin Plugin;

};

#endif // PLUGINSELECTIONBUTTON_H
