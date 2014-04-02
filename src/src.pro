include($$PWD/../unv.pri)

TEMPLATE = app
TARGET = unv
DESTDIR = $$OUTPUT_DIR/bin

DEPENDPATH += .
INCLUDEPATH += .

HEADERS += lexer.cpp \
           parser.cpp \
           ast.cpp

SOURCES += main.cpp \
           lexer.cpp \
           parser.cpp \
           ast.cpp

LIBS += $$system(llvm-config --ldflags --libs core)
QMAKE_CXXFLAGS += $$system(llvm-config --cppflags core)
