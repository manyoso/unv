lessThan(QT_MAJOR_VERSION, 5) {
   error(Qt version 5.2 or greater is required)
}

equals(QT_MAJOR_VERSION, 5) {
    lessThan(QT_MINOR_VERSION, 2) {
        error(Qt version 5.2 or greater is required)
    }
}

OUTPUT_DIR = $$(OUTPUT_DIR)
isEmpty(OUTPUT_DIR):OUTPUT_DIR=$$PWD/build

QT += core
QT -= gui

CONFIG += qt warn_on silent c++11

DEBUG_MODE = $$(DEBUG_MODE)
contains(DEBUG_MODE, 1) {
  CONFIG += debug
}

QMAKE_CXXFLAGS += -Wno-inconsistent-missing-override

OBJECTS_DIR = tmp
MOC_DIR = tmp

TOPLEVELDIR = $$PWD

unv.output = ${QMAKE_FILE_BASE}.o
unv.commands = $$OUTPUT_DIR/bin/unv $$UNV_INCLUDE -e obj ${QMAKE_FILE_NAME} -o ${QMAKE_FILE_OUT}
unv.input = UNV_SOURCES
unv.depends = $$OUTPUT_DIR/bin/unv
unv.variable_out = OBJECTS
QMAKE_EXTRA_COMPILERS += unv
