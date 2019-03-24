#-------------------------------------------------
#
# Project created by QtCreator 2018-12-19T11:49:20
#
#-------------------------------------------------

QT += core gui widgets concurrent dbus
TARGET = orson
TEMPLATE = app
CONFIG += c++1z

include(src/third-party/singleapplication/singleapplication.pri)

DEFINES += \
    QAPPLICATION_CLASS=QApplication \
    QT_DEPRECATED_WARNINGS

#DEFINES += KDE
contains(DEFINES, KDE){
    QT += KNotifications
}

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/tasksdialog.cpp \
    src/settingsdialog.cpp \
    src/autosynctimer.cpp \
    src/systemtray.cpp \
    src/pacman.cpp \
    src/pacmansettings.cpp \
    src/appsettings.cpp \
    src/packages-view/depend.cpp \
    src/packages-view/package.cpp \
    src/packages-view/packagesmodel.cpp \
    src/packages-view/packagesview.cpp \
    src/files-view/file.cpp \
    src/files-view/filesmodel.cpp \
    src/files-view/filesview.cpp \
    src/tasks-view/task.cpp \
    src/tasks-view/tasksmodel.cpp \
    src/tasks-view/tasksview.cpp

HEADERS += \
    src/mainwindow.h \
    src/tasksdialog.h \
    src/settingsdialog.h \
    src/autosynctimer.h \
    src/systemtray.h \
    src/pacman.h \
    src/pacmansettings.h \
    src/appsettings.h \
    src/packages-view/depend.h \
    src/packages-view/package.h \
    src/packages-view/packagesmodel.h \
    src/packages-view/packagesview.h \
    src/files-view/file.h \
    src/files-view/filesmodel.h \
    src/files-view/filesview.h \
    src/tasks-view/task.h \
    src/tasks-view/tasksmodel.h \
    src/tasks-view/tasksview.h

FORMS += \
    src/mainwindow.ui \
    src/tasksdialog.ui \
    src/settingsdialog.ui

LIBS += -lalpm

# Rules for deployment
bin.path = /usr/bin
bin.files = $${TARGET}

desktop.path = /usr/share/applications
desktop.files = dist/orson.desktop

sync-service.path = /usr/lib/systemd/system/
sync-service.files = dist/

sync-rule.path = /etc/polkit-1/rules.d/
sync-rule.files = dist/10-orson-sync.rules

INSTALLS += \
    bin \
    desktop \
    sync-service \
    sync-rule

# Check with PVS Studio
#CONFIG += pvs
CONFIG(pvs) {
    pvs_studio.target = $${TARGET}
    pvs_studio.sources = $${SOURCES}
    pvs_studio.output = true
    pvs_studio.cfg_text = "analysis-mode = 0"

    include(src/third-party/pvs-studio/PVS-Studio.pri)
}
