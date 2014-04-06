DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

HEADERS += $$PWD/ast.h \
           $$PWD/lexer.h \
           $$PWD/parser.h \
           $$PWD/sourcebuffer.h \
           $$PWD/token.h

SOURCES += $$PWD/ast.cpp \
           $$PWD/lexer.cpp \
           $$PWD/parser.cpp \

LIBS += $$system(llvm-config --ldflags --libs core)