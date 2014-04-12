#ifndef ast_h
#define ast_h

#include <QtCore>

#include "token.h"

// forward declarations
struct AliasDecl;
struct BinaryExpr;
struct Expr;
struct Stmt;
struct FuncCallExpr;
struct FuncDeclArg;
struct FuncDecl;
struct FuncStmt;
struct LiteralExpr;
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

struct BinaryExpr : public Expr {
    enum BinaryOp {
        Equality,
        LessThanOrEquality,
        GreaterThanOrEquality,
        Addition,
        Subtraction,
        Multiplication
    };
    BinaryOp op;
    QSharedPointer<Expr> lhs;
    QSharedPointer<Expr> rhs;
};

struct FuncCallExpr : public Expr {
    Token callee;
    QList<QSharedPointer<Expr> > args;
};

struct Stmt {
};

struct IfStmt : public Stmt {
    QSharedPointer<Expr> expr;
    QSharedPointer<Stmt> stmt;
};

struct ReturnStmt : public Stmt {
    QSharedPointer<Expr> expr;
};

struct FuncDeclArg {
    Token name;
    Token type;
};

struct FuncDef {
    QList<QSharedPointer<Stmt> > stmts;
};

struct FuncDecl {
    Token name;
    QList<QSharedPointer<FuncDeclArg> > args;
    Token returnType;
    QSharedPointer<FuncDef> funcDef;
};

struct VarExpr : public Expr {
    Token var;
};

struct LiteralExpr: public Expr {
    Token literal;
};

#endif // ast_h
