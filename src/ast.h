#ifndef ast_h
#define ast_h

#include <QtCore>

#include "token.h"
#include "typesystem.h"

// forward declarations
struct BinaryExpr;
struct Expr;
struct IfStmt;
struct IncludeDecl;
struct FuncCallExpr;
struct FuncDef;
struct FuncDecl;
struct LiteralExpr;
struct ReturnStmt;
struct Stmt;
struct TranslationUnit;
struct TypeCtorExpr;
struct TypeDecl;
struct TypeObject;
struct TypeParam;
struct VarExpr;
struct VarDeclStmt;
struct Visitor;

struct Node {
    enum Kind {
        _AliasDecl,
        _BinaryExpr,
        _BuiltinDecl,
        _IfStmt,
        _IncludeDecl,
        _FuncCallExpr,
        _FuncDef,
        _FuncDecl,
        _LiteralExpr,
        _ReturnStmt,
        _StructDecl,
        _TranslationUnit,
        _TypeCtorExpr,
        _TypeObject,
        _TypeParam,
        _VarDeclStmt,
        _VarExpr
    };

    QString kindToString() const
    {
        switch (kind) {
        case _AliasDecl:        return "AliasDecl";
        case _BinaryExpr:       return "BinaryExpr";
        case _BuiltinDecl:      return "BuiltinDecl";
        case _IfStmt:           return "IfStmt";
        case _IncludeDecl:      return "IncludeDecl";
        case _FuncCallExpr:     return "FuncCallExpr";
        case _FuncDef:          return "FuncDef";
        case _FuncDecl:         return "FuncDecl";
        case _LiteralExpr:      return "LiteralExpr";
        case _StructDecl:       return "StructDecl";
        case _ReturnStmt:       return "ReturnStmt";
        case _TranslationUnit:  return "TranslationUnit";
        case _TypeCtorExpr:     return "TypeCtorExpr";
        case _TypeObject:       return "TypeObject";
        case _TypeParam:        return "TypeParam";
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
    QList<QSharedPointer<IncludeDecl> > includeDecl;
    QList<QSharedPointer<TypeDecl> > typeDecl;
    QList<QSharedPointer<FuncDecl> > funcDecl;
    virtual void walk(Visitor&);
};

struct IncludeDecl : public Node {
    IncludeDecl() : Node(_IncludeDecl) {}
    Token include;
    virtual void walk(Visitor&);
};

struct TypeObject : public Node, public TypeRef {
    TypeObject() : Node(_TypeObject) {}
    Token name;
    Token type;
    virtual void walk(Visitor&);

    // inherited from TypeRef
    virtual QStringRef refName() const { return name.toStringRef(); }
    virtual QStringRef typeName() const { return type.toStringRef(); }
};

struct TypeParam : public Node {
    TypeParam() : Node(_TypeParam) {}
    Token name;
    virtual void walk(Visitor&);
};

struct TypeDecl : public Node, public TypeInfo {
    TypeDecl(Kind kind) : Node(kind) {}

    Token name;
    QString _namespace;
    QList<QSharedPointer<TypeObject> > objects;
    QList<QSharedPointer<TypeParam> > params;
    QList<Token> attributes;
    virtual void walk(Visitor&);

    // inherited from TypeInfo
    virtual QStringRef typeName() const { return name.toStringRef(); }
    virtual QString qualifiedTypeName() const { return _namespace + "::" + name.toString(); }
    virtual bool isNode() const { return true; }
    virtual bool isStructure() const { return kind == _StructDecl; }
    virtual bool isFunction() const { return kind == _FuncDecl; }
    virtual bool isAlias() const { return kind == _AliasDecl; }
    virtual QList<TypeRef*> typeRefList() const
    {
        QList<TypeRef*> list;
        foreach (QSharedPointer<TypeObject> obj, objects)
            list.append(obj.data());
        return list;
    }
};

struct Expr : public Node {
    Expr(Kind kind) : Node(kind) {}
    Token start;
};

struct BinaryExpr : public Expr {
    enum BinaryOp {
        OpEquality,
        OpNotEquality,
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
        case OpNotEquality:           return "!=";
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
    virtual void walk(Visitor&);
};

struct FuncDef : public Node {
    FuncDef() : Node(_FuncDef) {}
    QList<QSharedPointer<Stmt> > stmts;
    virtual void walk(Visitor&);
};

struct FuncDecl : public TypeDecl {
    FuncDecl() : TypeDecl(_FuncDecl) {}
    QSharedPointer<TypeObject> returnType;
    QSharedPointer<FuncDef> funcDef;
    QList<Token> attributes;
    virtual void walk(Visitor&);

    // inherited from TypeInfo
    virtual TypeRef* returnTypeRef() const { return returnType.data(); }
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
    QSharedPointer<TypeCtorExpr> expr;
    virtual void walk(Visitor&);
};

#endif // ast_h
