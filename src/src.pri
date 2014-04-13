DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

HEADERS += $$PWD/ast.h \
           $$PWD/astprinter.h \
           $$PWD/lexer.h \
           $$PWD/options.h \
           $$PWD/parser.h \
           $$PWD/sourcebuffer.h \
           $$PWD/symbols.h \
           $$PWD/token.h \
           $$PWD/visitor.h

SOURCES += $$PWD/ast.cpp \
           $$PWD/astprinter.cpp \
           $$PWD/lexer.cpp \
           $$PWD/options.cpp \
           $$PWD/parser.cpp \
           $$PWD/symbols.cpp

LIBS += $$system(llvm-config --ldflags --libs core)
