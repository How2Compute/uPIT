#include "pluginselectionbutton.h"
#include <qDebug>


PluginSelectionButton::PluginSelectionButton(QWidget *parent) : QWidget(parent)
{

}

PluginSelectionButton::PluginSelectionButton(UnrealPlugin Plugin, MainWindow *window)
{
     button = new QPushButton(Plugin.GetName(), this);
     QObject::connect(button, SIGNAL(clicked(bool)), this, SLOT(onPluginSelected(bool)));
     // TODO it seems this class is successfully created, and the button is too, but the slot won't be found (QObject::connect: No such slot PluginSelectionButton::onPluginSelected() in ..\uPIT\pluginselectionbutton.cpp:13)
     //QObject::connect(button, SIGNAL(clicked(bool)), this, &PluginSelectionButton::onPluginSelected);
     this->Plugin = Plugin;
     mWindow = window;
}

void PluginSelectionButton::onPluginSelected(bool checked)
{
#ifdef QT_DEBUG
    qDebug() << "Plugin Button Pressed For Plugin With Friendly Name: " << Plugin.GetName();
#endif
    mWindow->SetPlugin(Plugin);
}
