include($$PWD/../unv.pri)
include($$PWD/../src/src.pri)

TEMPLATE = app
TARGET = unvtests
DESTDIR = $$OUTPUT_DIR/bin
CONFIG += qtestlib

DEPENDPATH += .
INCLUDEPATH += .

HEADERS += testlexer.h
SOURCES += main.cpp testlexer.cpp
