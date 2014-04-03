DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

HEADERS += $$PWD/lexer.h \
           $$PWD/parser.h \
           $$PWD/ast.h

SOURCES += $$PWD/lexer.cpp \
           $$PWD/parser.cpp \
           $$PWD/ast.cpp

LIBS += $$system(llvm-config --ldflags --libs core)
QMAKE_CXXFLAGS += $$system(llvm-config --cppflags core)
