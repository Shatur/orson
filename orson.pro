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
        src/packages-view/packagestreeview.cpp \
        src/packages-view/packagesmodel.cpp \
        src/packages-view/package.cpp \
        src/files-view/filestreeview.cpp \
        src/files-view/filesmodel.cpp \
        src/files-view/file.cpp

HEADERS += \
        src/mainwindow.h \
        src/packagemanager.h \
        src/packages-view/packagestreeview.h \
        src/packages-view/packagesmodel.h \
        src/packages-view/package.h \
        src/files-view/filestreeview.h \
        src/files-view/filesmodel.h \
        src/files-view/file.h

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
