#include "pluginselectionbutton.h"
#include <qDebug>


PluginSelectionButton::PluginSelectionButton(QWidget *parent) : QWidget(parent)
{

}

PluginSelectionButton::PluginSelectionButton(UnrealPlugin Plugin, MainWindow *window)
{
     button = new QPushButton(Plugin.GetName(), this);
     QObject::connect(button, SIGNAL(clicked(bool)), SLOT(onPluginSelected()));
     mWindow = window;
}

void PluginSelectionButton::onPluginSelected(bool checked)
{
#ifdef QT_DEBUG
    qDebug() << "Plugin Button Pressed For Plugin With Friendly Name: " << Plugin.GetName();
#endif
}
