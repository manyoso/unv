#ifndef ast_h
#define ast_h

#include <QtCore>

#include "token.h"

// forward declarations
struct AliasDecl;
struct Expr;
struct ExprStmt;
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

struct ExprStmt {
    QSharedPointer<Expr> expr;
};

struct IfStmt : public ExprStmt {
    QSharedPointer<ExprStmt> exprStmt;
};

struct ReturnStmt : public ExprStmt {
};

struct FuncDeclArg {
    Token name;
    Token type;
};

struct FuncStmt {
    QList<QSharedPointer<ExprStmt> > stmts;
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
