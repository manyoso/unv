#include "ast.h"
#include "visitor.h"

void AliasDecl::walk(Visitor& visitor)
{
    visitor.begin(*this);
    visitor.visit(*this);
    visitor.end(*this);
}

void BinaryExpr::walk(Visitor& visitor)
{
    visitor.begin(*this);
    visitor.visit(*this);
    lhs->walk(visitor);
    rhs->walk(visitor);
    visitor.end(*this);
}

void IfStmt::walk(Visitor& visitor)
{
    visitor.begin(*this);
    visitor.visit(*this);
    expr->walk(visitor);
    stmt->walk(visitor);
    visitor.end(*this);
}

void FuncCallExpr::walk(Visitor& visitor)
{
    visitor.begin(*this);
    visitor.visit(*this);
    foreach (QSharedPointer<Expr> arg, args)
        arg->walk(visitor);
    visitor.end(*this);
}

void FuncDef::walk(Visitor& visitor)
{
    visitor.begin(*this);
    visitor.visit(*this);
    foreach (QSharedPointer<Stmt> stmt, stmts)
        stmt->walk(visitor);
    visitor.end(*this);
}

void FuncDeclArg::walk(Visitor& visitor)
{
    visitor.begin(*this);
    visitor.visit(*this);
    visitor.end(*this);
}

void FuncDecl::walk(Visitor& visitor)
{
    visitor.begin(*this);
    visitor.visit(*this);
    foreach (QSharedPointer<FuncDeclArg> arg, args)
        arg->walk(visitor);
    funcDef->walk(visitor);
    visitor.end(*this);
}

void ReturnStmt::walk(Visitor& visitor)
{
    visitor.begin(*this);
    visitor.visit(*this);
    expr->walk(visitor);
    visitor.end(*this);
}

void LiteralExpr::walk(Visitor& visitor)
{
    visitor.begin(*this);
    visitor.visit(*this);
    visitor.end(*this);
}

void TranslationUnit::walk(Visitor& visitor)
{
    visitor.begin(*this);
    visitor.visit(*this);
    foreach (QSharedPointer<AliasDecl> alias, aliasDecl)
        alias->walk(visitor);
    foreach (QSharedPointer<FuncDecl> func, funcDecl)
        func->walk(visitor);
    visitor.end(*this);
}

void TypeDecl::walk(Visitor& visitor)
{
    visitor.begin(*this);
    visitor.visit(*this);
    visitor.end(*this);
}

void VarExpr::walk(Visitor& visitor)
{
    visitor.begin(*this);
    visitor.visit(*this);
    visitor.end(*this);
}

void VarDeclStmt::walk(Visitor& visitor)
{
    visitor.begin(*this);
    visitor.visit(*this);
    expr->walk(visitor);
    visitor.end(*this);
}