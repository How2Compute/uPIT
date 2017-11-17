#include "pluginselectionbutton.h"
#include <qDebug>


PluginSelectionButton::PluginSelectionButton(QWidget *parent) : QWidget(parent)
{

}

PluginSelectionButton::PluginSelectionButton(UnrealPlugin Plugin, MainWindow *window)
{
    // Create a new push button as a sort of child and bind it to call the onPluginSelected function when clicked.
    button = new QPushButton(Plugin.GetName(), this);
    QObject::connect(button, SIGNAL(clicked(bool)), this, SLOT(onPluginSelected(bool)));

    // Set the class's values based upon the parameters so we can later reference these
    this->Plugin = Plugin;
    mWindow = window;
}

void PluginSelectionButton::onPluginSelected(bool checked)
{
#ifdef QT_DEBUG
    qDebug() << "Plugin Button Pressed For Plugin With Friendly Name: " << Plugin.GetName();
#endif
    if (mWindow)
    {
        mWindow->SetPlugin(Plugin);
    }
#ifdef QT_DEBUG
    else
    {
        qDebug() << "Invalid Window Pointer onPluginSelected For Plugin " << Plugin.GetName();
    }
#endif
}
