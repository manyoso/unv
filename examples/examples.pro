include($$PWD/../unv.pri)

TEMPLATE = app
TARGET = fibonacci
DESTDIR = $$OUTPUT_DIR/bin/examples

UNV_SOURCES += $$PWD/fibonacci.unv

unv.output = ${QMAKE_FILE_BASE}.o
unv.commands = $$OUTPUT_DIR/bin/unv -e obj ${QMAKE_FILE_NAME} -o ${QMAKE_FILE_OUT}
unv.input = UNV_SOURCES
unv.depends = $$OUTPUT_DIR/bin/unv
QMAKE_EXTRA_COMPILERS += unv
