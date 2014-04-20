DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

HEADERS += $$PWD/ast.h \
           $$PWD/astprinter.h \
           $$PWD/codegen.h \
           $$PWD/lexer.h \
           $$PWD/options.h \
           $$PWD/output.h \
           $$PWD/parser.h \
           $$PWD/sourcebuffer.h \
           $$PWD/typesystem.h \
           $$PWD/token.h \
           $$PWD/visitor.h

SOURCES += $$PWD/ast.cpp \
           $$PWD/astprinter.cpp \
           $$PWD/codegen.cpp \
           $$PWD/lexer.cpp \
           $$PWD/options.cpp \
           $$PWD/output.cpp \
           $$PWD/parser.cpp \
           $$PWD/typesystem.cpp

QMAKE_CXXFLAGS += $$system(llvm-config --cppflags)
LIBS += $$system(llvm-config --ldflags --libs core)
