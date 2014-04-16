include($$PWD/../unv.pri)
include($$PWD/../src/src.pri)

TEMPLATE = app
TARGET = unvtests
DESTDIR = $$OUTPUT_DIR/bin
QT += testlib

DEPENDPATH += .
INCLUDEPATH += .

HEADERS += testerrors.h \
           testexamples.h \
           testlexer.h \
           testparser.h

SOURCES += main.cpp \
           testerrors.cpp \
           testexamples.cpp \
           testlexer.cpp \
           testparser.cpp

RESOURCES = tests.qrc
