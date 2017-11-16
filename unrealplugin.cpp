#include "unrealplugin.h"

/*
UnrealPlugin::UnrealPlugin(QObject *parent) : QObject(parent)
{

}


UnrealPlugin::UnrealPlugin(QString Name, QString Path, QString Description, QString EngineVersion, bool Installed)
{
    PluginName = Name;
    PluginPath = Path;
    PluginDescription = Description;
    UnrealEngineVersion = EngineVersion;
    bInstalled = Installed;
}
*/

UnrealPlugin::UnrealPlugin(QString Name, QString Path, QString Description, QString EngineVersion, PluginSource Source, bool Installed)
{
    PluginName = Name;
    PluginPath = Path;
    PluginDescription = Description;
    UnrealEngineVersion = EngineVersion;
    PluginOrigin = Source;
    bInstalled = Installed;
}

UnrealPlugin::UnrealPlugin()
{
    // Initialize a (default) blank plugin
    UnrealPlugin("NO_PLUGIN_SELECTED", "", "", "", PluginSource::PS_OTHER, false);
}

void UnrealPlugin::SetName(QString Name)
{
    PluginName = Name;
}

void UnrealPlugin::SetPath(QString Path)
{
    PluginPath = Path;
}

void UnrealPlugin::SetDescription(QString Description)
{
    PluginDescription = Description;
}

void UnrealPlugin::SetInstalled(bool IsInstalled)
{
    bInstalled = IsInstalled;
}

void UnrealPlugin::SetEngine(QString Version)
{
    UnrealEngineVersion = Version;
}

void UnrealPlugin::SetPluginSource(PluginSource Source)
{
    PluginOrigin = Source;
}

QString UnrealPlugin::GetName()
{
    return PluginName;
}

QString UnrealPlugin::GetPath()
{
    return PluginPath;
}

QString UnrealPlugin::GetDescription()
{
    return PluginDescription;
}

bool UnrealPlugin::GetInstalled()
{
    return bInstalled;
}

QString UnrealPlugin::GetEngineVersion()
{
    return UnrealEngineVersion;
}

PluginSource UnrealPlugin::GetPluginSource()
{
    return PluginOrigin;
}

bool UnrealPlugin::operator==(const UnrealPlugin &other) const
{
    return PluginName == other.PluginName && PluginPath == other.PluginPath && UnrealEngineVersion == other.UnrealEngineVersion && bInstalled == other.bInstalled;
}

UnrealPlugin UnrealPlugin::NoPlugin = UnrealPlugin("NO_PLUGIN_SELECTED", "", "", "", PluginSource::PS_OTHER, false);
