#ifndef ast_h
#define ast_h

#include <QtCore>

#include "token.h"

// forward declarations
struct AliasDecl;
struct BinaryExpr;
struct Expr;
struct IfStmt;
struct Stmt;
struct FuncCallExpr;
struct FuncDef;
struct FuncDeclArg;
struct FuncDecl;
struct ReturnStmt;
struct LiteralExpr;
struct TranslationUnit;
struct VarExpr;
struct Visitor;

struct Node {
    enum Type {
        _AliasDecl,
        _BinaryExpr,
        _IfStmt,
        _FuncCallExpr,
        _FuncDef,
        _FuncDeclArg,
        _FuncDecl,
        _LiteralExpr,
        _ReturnStmt,
        _TranslationUnit,
        _VarExpr
    };

    QString typeToString() const
    {
        switch (type) {
        case _AliasDecl:        return "AliasDecl";
        case _BinaryExpr:       return "BinaryExpr";
        case _IfStmt:           return "IfStmt";
        case _FuncCallExpr:     return "FuncCallExpr";
        case _FuncDef:          return "FuncDef";
        case _FuncDeclArg:      return "FuncDeclArg";
        case _FuncDecl:         return "FuncDecl";
        case _LiteralExpr:      return "LiteralExpr";
        case _ReturnStmt:       return "ReturnStmt";
        case _TranslationUnit:  return "TranslationUnit";
        case _VarExpr:          return "VarExpr";
        default:
            Q_ASSERT(false); // should never be reached
            break;
        }
    }

    Node(Type type) : type(type) {}
    virtual ~Node() {}

    virtual void walk(Visitor&) = 0;

    Type type;
};

struct TranslationUnit : public Node {
    TranslationUnit() : Node(_TranslationUnit) {}
    QList<QSharedPointer<AliasDecl> > aliasDecl;
    QList<QSharedPointer<FuncDecl> > funcDecl;
    virtual void walk(Visitor&);
};

struct AliasDecl : public Node {
    AliasDecl() : Node(_AliasDecl) {}
    Token type;
    Token alias;
    virtual void walk(Visitor&);
};

struct Expr : public Node {
    Expr(Type type) : Node(type) {}
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
    BinaryExpr() : Expr(_BinaryExpr) {}
    BinaryOp op;
    QSharedPointer<Expr> lhs;
    QSharedPointer<Expr> rhs;
    virtual void walk(Visitor&);
};

struct FuncCallExpr : public Expr {
    FuncCallExpr() : Expr(_FuncCallExpr) {}
    Token callee;
    QList<QSharedPointer<Expr> > args;
    virtual void walk(Visitor&);
};

struct Stmt : public Node {
    Stmt(Type type) : Node(type) {}
};

struct IfStmt : public Stmt {
    IfStmt() : Stmt(_IfStmt) {}
    QSharedPointer<Expr> expr;
    QSharedPointer<Stmt> stmt;
    virtual void walk(Visitor&);
};

struct ReturnStmt : public Stmt {
    ReturnStmt() : Stmt(_ReturnStmt) {}
    QSharedPointer<Expr> expr;
    virtual void walk(Visitor&);
};

struct FuncDeclArg : public Node {
    FuncDeclArg() : Node(_FuncDeclArg) {}
    Token name;
    Token type;
    virtual void walk(Visitor&);
};

struct FuncDef : public Node {
    FuncDef() : Node(_FuncDef) {}
    QList<QSharedPointer<Stmt> > stmts;
    virtual void walk(Visitor&);
};

struct FuncDecl : public Node {
    FuncDecl() : Node(_FuncDecl) {}
    Token name;
    QList<QSharedPointer<FuncDeclArg> > args;
    Token returnType;
    QSharedPointer<FuncDef> funcDef;
    virtual void walk(Visitor&);
};

struct VarExpr : public Expr {
    VarExpr() : Expr(_VarExpr) {}
    Token var;
    virtual void walk(Visitor&);
};

struct LiteralExpr: public Expr {
    LiteralExpr() : Expr(_LiteralExpr) {}
    Token literal;
    virtual void walk(Visitor&);
};

#endif // ast_h
