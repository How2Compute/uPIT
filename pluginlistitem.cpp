#include "pluginlistitem.h"

PluginListItem::PluginListItem(UnrealPlugin Plugin, QListWidget *parent)
{
    // Create a new list widget item that has the plugin's name.
    new QListWidgetItem(Plugin.GetName());

    // Store the plugin so it can later be retrieved.
    this->Plugin = Plugin;
}

UnrealPlugin PluginListItem::GetPlugin()
{
    return Plugin;
}
