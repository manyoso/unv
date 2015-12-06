UNV_INCLUDE = --include ../../core
UNV_SOURCES += $$PWD/helloworld.unv

include($$PWD/../unv.pri)

TEMPLATE = app
TARGET = helloworld
DESTDIR = $$OUTPUT_DIR/bin/examples
