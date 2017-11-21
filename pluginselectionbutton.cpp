#include "pluginselectionbutton.h"

PluginSelectionButton::PluginSelectionButton(UnrealPlugin Plugin, QListWidget *parent)
{
    // Create a new list widget item, and keep track of the plugin so this can later be retrieved.
    button = new QListWidgetItem(Plugin.GetName());//, parent);

    this->Plugin = Plugin;
}

UnrealPlugin PluginSelectionButton::GetPlugin()
{
    return Plugin;
}
