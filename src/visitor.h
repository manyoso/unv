#ifndef visitor_h
#define visitor_h

#include <QtCore>

#include "ast.h"

struct Visitor {
    void walk(Node& node) { node.walk(*this); }
    virtual void begin(Node&) = 0;
    virtual void end(Node&) = 0;

    virtual void visit(BinaryExpr&) {}
    virtual void visit(IfStmt&) {}
    virtual void visit(IncludeDecl&) {}
    virtual void visit(FuncCallExpr&) {}
    virtual void visit(FuncDef&) {}
    virtual void visit(FuncDecl&) {}
    virtual void visit(LiteralExpr&) {}
    virtual void visit(ReturnStmt&) {}
    virtual void visit(TranslationUnit&) {}
    virtual void visit(TypeCtorExpr&) {}
    virtual void visit(TypeDecl&) {}
    virtual void visit(TypeObject&) {}
    virtual void visit(TypeParam&) {}
    virtual void visit(VarExpr&) {}
    virtual void visit(VarDeclStmt&) {}
};

#endif // visitor_h
