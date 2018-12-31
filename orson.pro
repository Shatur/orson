#-------------------------------------------------
#
# Project created by QtCreator 2018-12-19T11:49:20
#
#-------------------------------------------------

QT += core gui widgets
TARGET = orson
TEMPLATE = app
CONFIG += c++1z

include(src/third-party/singleapplication/singleapplication.pri)

DEFINES += \
    QAPPLICATION_CLASS=QApplication \
    QT_DEPRECATED_WARNINGS

SOURCES += \
        src/main.cpp \
        src/mainwindow.cpp \
        src/package.cpp \
        src/packagemanager.cpp \
        src/files-view/filesystemmodel.cpp \
        src/files-view/filesystemitem.cpp \
        src/files-view/filestreeview.cpp

HEADERS += \
        src/mainwindow.h \
        src/package.h \
        src/packagemanager.h \
        src/files-view/filesystemmodel.h \
        src/files-view/filesystemitem.h \
        src/files-view/filestreeview.h

FORMS += \
        src/mainwindow.ui

LIBS += -lalpm

# Default rules for deployment
bin.path = /usr/bin
bin.files = $${TARGET}
INSTALLS += bin

# Check with PVS Studio
#CONFIG += pvs
CONFIG(pvs) {
    pvs_studio.target = $${TARGET}
    pvs_studio.sources = $${SOURCES}
    pvs_studio.output = true
    pvs_studio.cfg_text = "analysis-mode = 0"

    include(src/third-party/pvs-studio/PVS-Studio.pri)
}
