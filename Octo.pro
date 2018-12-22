#-------------------------------------------------
#
# Project created by QtCreator 2018-12-19T11:49:20
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Octo
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

CONFIG += c++1z

SOURCES += \
        src/main.cpp \
        src/mainwindow.cpp \
        src/package.cpp \
        src/packagemanager.cpp \
        src/filestreewidget.cpp

HEADERS += \
        src/mainwindow.h \
        src/package.h \
        src/packagemanager.h \
        src/filestreewidget.h

FORMS += \
        src/mainwindow.ui

LIBS += -lalpm

# Default rules for deployment.
bin.path = /usr/bin
bin.files = $${TARGET}

INSTALLS += bin
