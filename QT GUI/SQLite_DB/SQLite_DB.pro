#-------------------------------------------------
#
# Project created by QtCreator 2015-10-04T13:25:00
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SQLite_DB
TEMPLATE = app


SOURCES += main.cpp\
        login.cpp \
    myaccount.cpp

HEADERS  += login.h \
    myaccount.h

FORMS    += login.ui \
    myaccount.ui
