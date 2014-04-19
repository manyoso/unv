#ifndef ast_h
#define ast_h

#include <QtCore>

#include "token.h"

// forward declarations
struct BinaryExpr;
struct Expr;
struct IfStmt;
struct FuncCallExpr;
struct FuncDef;
struct FuncDecl;
struct ReturnStmt;
struct LiteralExpr;
struct Stmt;
struct TranslationUnit;
struct TypeCtorExpr;
struct TypeDecl;
struct TypeObject;
struct VarExpr;
struct VarDeclStmt;
struct Visitor;

class TypeHandle;

struct Node {
    enum Kind {
        _BinaryExpr,
        _IfStmt,
        _FuncCallExpr,
        _FuncDef,
        _FuncDecl,
        _LiteralExpr,
        _ReturnStmt,
        _TranslationUnit,
        _TypeCtorExpr,
        _TypeDecl,
        _TypeObject,
        _VarDeclStmt,
        _VarExpr
    };

    QString kindToString() const
    {
        switch (kind) {
        case _BinaryExpr:       return "BinaryExpr";
        case _IfStmt:           return "IfStmt";
        case _FuncCallExpr:     return "FuncCallExpr";
        case _FuncDef:          return "FuncDef";
        case _FuncDecl:         return "FuncDecl";
        case _LiteralExpr:      return "LiteralExpr";
        case _ReturnStmt:       return "ReturnStmt";
        case _TranslationUnit:  return "TranslationUnit";
        case _TypeCtorExpr:         return "TypeCtorExpr";
        case _TypeDecl:         return "TypeDecl";
        case _TypeObject:       return "TypeObject";
        case _VarDeclStmt:      return "VarDeclStmt";
        case _VarExpr:          return "VarExpr";
        }
    }

    Node(Kind kind) : kind(kind) {}
    virtual ~Node() {}

    virtual void walk(Visitor&) = 0;

    Kind kind;
};

struct TranslationUnit : public Node {
    TranslationUnit() : Node(_TranslationUnit) {}
    QList<QSharedPointer<TypeDecl> > typeDecl;
    QList<QSharedPointer<FuncDecl> > funcDecl;
    virtual void walk(Visitor&);
};

struct TypeDecl : public Node {
    TypeDecl() : Node(_TypeDecl) {}
    Token name;
    QList<QSharedPointer<TypeObject> > objects;
    virtual void walk(Visitor&);
};

struct Expr : public Node {
    Expr(Kind kind) : Node(kind) {}
    Token start;
};

struct BinaryExpr : public Expr {
    enum BinaryOp {
        OpEquality,
        OpLessThanOrEquality,
        OpGreaterThanOrEquality,
        OpLessThan,
        OpGreaterThan,
        OpAddition,
        OpSubtraction,
        OpMultiplication,
        OpDivision
    };

    QString opToString() const
    {
        switch (op) {
        case OpEquality:              return "==";
        case OpLessThanOrEquality:    return "<=";
        case OpGreaterThanOrEquality: return ">=";
        case OpLessThan:              return "<";
        case OpGreaterThan:           return ">";
        case OpAddition:              return "+";
        case OpSubtraction:           return "-";
        case OpMultiplication:        return "*";
        case OpDivision:              return "/";
        }
    }

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

struct TypeCtorExpr : public Expr {
    TypeCtorExpr() : Expr(_TypeCtorExpr) {}
    Token type;
    QList<QSharedPointer<Expr> > args;
    virtual void walk(Visitor&);
};

struct Stmt : public Node {
    Stmt(Kind kind) : Node(kind) {}
};

struct IfStmt : public Stmt {
    IfStmt() : Stmt(_IfStmt) {}
    QSharedPointer<Expr> expr;
    QSharedPointer<Stmt> stmt;
    virtual void walk(Visitor&);
};

struct ReturnStmt : public Stmt {
    ReturnStmt() : Stmt(_ReturnStmt) {}
    Token keyword;
    QSharedPointer<Expr> expr;
    TypeHandle* typeHandle;
    virtual void walk(Visitor&);
};

struct TypeObject : public Node {
    TypeObject() : Node(_TypeObject) {}
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
    QList<QSharedPointer<TypeObject> > objects;
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

struct VarDeclStmt: public Stmt {
    VarDeclStmt() : Stmt(_VarDeclStmt) {}
    Token type;
    Token name;
    QSharedPointer<Expr> expr;
    virtual void walk(Visitor&);
};

#endif // ast_h
