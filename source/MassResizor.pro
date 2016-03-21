#-------------------------------------------------
#
# Project created by QtCreator 2014-04-20T02:45:34
#
#-------------------------------------------------

CONFIG = c++11

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MassResizor
TEMPLATE = app


SOURCES += main.cpp\
        dialog.cpp

HEADERS  += dialog.h

FORMS    += dialog.ui


LIBS += -lQtUIUtils
