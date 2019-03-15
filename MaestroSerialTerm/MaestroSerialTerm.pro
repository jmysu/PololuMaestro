#-------------------------------------------------
#
# Project created by QtCreator 2018-10-27T17:36:49
#
#-------------------------------------------------

QT       += core gui
QT       += serialport

RC_FILE = resourcecollection.rc

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MaestroSerialTerm
TEMPLATE = app
#-------------------------------------------------
VERSION = 0.9.2    # major.minor.patch
# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += APP_NAME=\"\\\"$$TARGET\\\"\"
DEFINES += APP_VERSION=\"\\\"$$VERSION\\\"\"
# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    mainwindow_settings.cpp \
    mainwindow_serial.cpp \
    mainwindow_maestrocmd.cpp \
    mainwindow_maestroui.cpp

HEADERS += \
    mainwindow.h \
    constants.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
