#-------------------------------------------------
#
# Project created by QtCreator 2017-11-15T21:11:02
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = uPIT
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        Source/main.cpp \
        Source/Widgets/mainwindow.cpp \
    Source/DataTypes/unrealplugin.cpp \
    Source/DataTypes/unrealinstall.cpp \
    Source/DataTypes/pluginsource.cpp \
    Source/Widgets/pluginlistitem.cpp

HEADERS += \
        Source/Widgets/mainwindow.h \
    Source/DataTypes/unrealplugin.h \
    Source/DataTypes/unrealinstall.h \
    Source/DataTypes/pluginsource.h \
    Source/Widgets/pluginlistitem.h

FORMS += \
        Source/Widgets/mainwindow.ui

# Allows us to use nullptr/other c++11 stuff
QMAKE_CXXFLAGS += -std=c++0x
