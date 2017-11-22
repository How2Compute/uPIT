#ifndef PLUGINSELECTIONBUTTON_H
#define PLUGINSELECTIONBUTTON_H

#include "Source/DataTypes/unrealplugin.h"
#include <QListWidgetItem>

class PluginListItem : public QListWidgetItem
{
public:
    // Construct a plugin selection list item with the given plugin. NOTE: Does NOT add it to the parent.
    PluginListItem(UnrealPlugin Plugin, QListWidget *parent);

    // Returns the plugin that this item was created with.
    UnrealPlugin GetPlugin();

private:
    // The plugin this button/item is associated with.
    UnrealPlugin Plugin;

};

#endif // PLUGINSELECTIONBUTTON_H
