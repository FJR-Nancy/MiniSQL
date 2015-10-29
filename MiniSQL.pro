#-------------------------------------------------
#
# Project created by QtCreator 2013-10-21T00:58:24
#
#-------------------------------------------------

QT       += core gui
QT       += webkitwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MiniSQL
TEMPLATE = app


SOURCES += main.cpp\
        minisql.cpp \
    texteditarea.cpp \
    codehighlighter.cpp \
    finddialog.cpp \
    replacedialog.cpp \
    gotodialog.cpp \
    linkdialog.cpp \
    Interpreter.cpp \
    CatalogManager.cpp \
    BufferManager.cpp \
    BPlusTreeIndex.cpp \
    APIManager.cpp \
    RecordManager.cpp \
    tabledialog.cpp

HEADERS  += minisql.h \
    texteditarea.h \
    codehighlighter.h \
    finddialog.h \
    replacedialog.h \
    gotodialog.h \
    linkdialog.h \
    Main.h \
    Interpreter.h \
    CatalogManager.h \
    BufferManager.h \
    BPlusTreeIndex.h \
    APIManager.h \
    TreeNode.h \
    RecordManager.h \
    tabledialog.h

FORMS    += minisql.ui \
    finddialog.ui \
    replacedialog.ui \
    gotodialog.ui \
    linkdialog.ui \
    tabledialog.ui

RESOURCES += \
    image.qrc

RC_FILE = MiniSQL.rc


