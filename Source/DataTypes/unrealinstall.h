#ifndef UNREALINSTALL_H
#define UNREALINSTALL_H

#include <QObject>

class UnrealInstall
{
public:
    UnrealInstall();

    // Set up a binary build of unreal engine
    UnrealInstall(QString Name, QString Path);

    // Set up a source build of unreal engine
    UnrealInstall(QString Path);

    void SetName(QString Name);
    void SetPath(QString Path);

    QString GetName();
    QString GetPath();

    bool operator==(const UnrealInstall &other) const;

private:
    QString EngineName;
    QString EnginePath;
};

#endif // UNREALINSTALL_H
