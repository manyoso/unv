UNV_INCLUDE = --include ../../core
UNV_SOURCES += $$PWD/fibonacci.unv

include($$PWD/../unv.pri)

TEMPLATE = app
TARGET = fibonacci
DESTDIR = $$OUTPUT_DIR/bin/examples
