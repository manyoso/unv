#ifndef ast_h
#define ast_h

#include <QtCore>

#include "token.h"

// forward declarations
struct AliasDecl;
struct Expr;
struct FuncCallExpr;
struct FuncDeclArg;
struct FuncDecl;
struct FuncStmt;
struct VarExpr;

struct TranslationUnit {
    QList<QSharedPointer<AliasDecl> > aliasDecl;
    QList<QSharedPointer<FuncDecl> > funcDecl;
};

struct AliasDecl {
    Token type;
    Token alias;
};

struct Expr {
    Token type;
};

struct FuncCallExpr : public Expr {
    Token callee;
    QList<QSharedPointer<Expr> > args;
};

struct ReturnExpr : public Expr {
    QSharedPointer<Expr> expr;
};

struct FuncDeclArg {
    Token name;
    Token type;
};

struct FuncStmt {
    QSharedPointer<Expr> expr;
};

struct FuncDecl {
    Token name;
    QList<QSharedPointer<FuncDeclArg> > args;
    QSharedPointer<FuncStmt> stmt;
    Token returnType;
};

struct VarExpr : public Expr {
    Token var;
};

#endif // ast_h
