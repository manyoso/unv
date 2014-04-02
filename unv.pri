OUTPUT_DIR = $$(OUTPUT_DIR)
isEmpty(OUTPUT_DIR):OUTPUT_DIR=$$PWD/build

QT += core

CONFIG += qt warn_on

#DEBUG_MODE = $$(DEBUG_MODE)
#contains(DEBUG_MODE, 1) {
  CONFIG += debug
#}

OBJECTS_DIR = tmp
MOC_DIR = tmp

TOPLEVELDIR = $$PWD

