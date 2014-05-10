UNV_INCLUDE = --include ../../core
UNV_SOURCES += $$PWD/numericliterals.unv

include($$PWD/../unv.pri)

TEMPLATE = app
TARGET = numericliterals
DESTDIR = $$OUTPUT_DIR/bin/examples
