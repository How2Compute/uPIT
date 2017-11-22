#ifndef UNREALPLUGIN_H
#define UNREALPLUGIN_H

#include <QObject>
#include "pluginsource.h"

class UnrealPlugin// : public QObject
{
public:
    UnrealPlugin();
    UnrealPlugin(QString Name, QString Path, QString Description, QString EngineVersion, PluginSource Source, bool Installed);

    void SetName(QString Name);
    void SetPath(QString Path);
    void SetDescription(QString Description);
    void SetInstalled(bool IsInstalled);
    void SetEngine(QString Version);
    void SetPluginSource(PluginSource Source);

    QString GetName();
    QString GetPath();
    QString GetDescription();
    bool GetInstalled();
    QString GetEngineVersion();
    PluginSource GetPluginSource();

    static UnrealPlugin NoPlugin;

    bool operator==(const UnrealPlugin &other) const;

signals:

public slots:

private:
    QString PluginName;
    QString PluginPath;
    QString PluginDescription;

    bool bInstalled;

    QString UnrealEngineVersion;    // TODO find a neat way to handle this (on windows there is that file)

    PluginSource PluginOrigin;
};

#endif // UNREALPLUGIN_H
