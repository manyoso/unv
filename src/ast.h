#ifndef ast_h
#define ast_h

#include <QtCore>

#include "token.h"

// forward declarations
struct AliasDecl;
struct TypeDeclArg;
struct TypeDecl;

struct TranslationUnit {
    ~TranslationUnit()
    { qDeleteAll(aliasDecl); qDeleteAll(typeDecl); }
    QList<AliasDecl*> aliasDecl;
    QList<TypeDecl*> typeDecl;
};

struct AliasDecl {
    Token type;
    Token alias;
};

struct TypeDeclArg {
    Token name;
    Token type;
};

struct TypeDecl {
    ~TypeDecl()
    { qDeleteAll(args); }
    Token type;
    QList<TypeDeclArg*> args;
    Token returnType;
};

#endif // ast_h
