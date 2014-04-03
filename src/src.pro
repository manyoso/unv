include($$PWD/../unv.pri)

TEMPLATE = app
TARGET = unv
DESTDIR = $$OUTPUT_DIR/bin

DEPENDPATH += .
INCLUDEPATH += .

SOURCES += main.cpp

include($$PWD/src.pri)
