include($$PWD/../unv.pri)
include($$PWD/../src/src.pri)

TEMPLATE = app
TARGET = unvtests
DESTDIR = $$OUTPUT_DIR/bin
QT += testlib

DEPENDPATH += .
INCLUDEPATH += .

HEADERS += testexamples.h \
           testlexer.h
SOURCES += main.cpp \
           testexamples.cpp \
           testlexer.cpp
