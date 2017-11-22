#include "unrealinstall.h"


UnrealInstall::UnrealInstall()
{
    UnrealInstall("NO_NAME_SPECIFIED", "NO_PATH_SPECIFIED");
}

UnrealInstall::UnrealInstall(QString Name, QString Path)
{
    EngineName = Name;
    EnginePath = Path;
}

UnrealInstall::UnrealInstall(QString Path)
{
    EngineName = "SOURCE_BUILD";
    EnginePath = Path;
}

void UnrealInstall::SetName(QString Name)
{
    EngineName = Name;
}

void UnrealInstall::SetPath(QString Path)
{
    EnginePath = Path;
}

QString UnrealInstall::GetName()
{
    return EngineName;
}

QString UnrealInstall::GetPath()
{
    return EnginePath;
}

bool UnrealInstall::operator==(const UnrealInstall &other) const
{
    // We only care about path name as the name is (mainly, if not only) there for cosmetic/display purposes
    return EnginePath == other.EnginePath;
}
