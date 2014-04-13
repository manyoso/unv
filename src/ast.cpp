#include "ast.h"
#include "visitor.h"

void AliasDecl::walk(Visitor& visitor)
{
    visitor.visit(*this);
}

void BinaryExpr::walk(Visitor& visitor)
{
    visitor.visit(*this);
    lhs->walk(visitor);
    rhs->walk(visitor);
}

void IfStmt::walk(Visitor& visitor)
{
    visitor.visit(*this);
    expr->walk(visitor);
    stmt->walk(visitor);
}

void FuncCallExpr::walk(Visitor& visitor)
{
    visitor.visit(*this);
    foreach (QSharedPointer<Expr> arg, args)
        arg->walk(visitor);
}

void FuncDef::walk(Visitor& visitor)
{
    visitor.visit(*this);
    foreach (QSharedPointer<Stmt> stmt, stmts)
        stmt->walk(visitor);
}

void FuncDeclArg::walk(Visitor& visitor)
{
    visitor.visit(*this);
}

void FuncDecl::walk(Visitor& visitor)
{
    visitor.visit(*this);

    foreach (QSharedPointer<FuncDeclArg> arg, args)
        arg->walk(visitor);

    funcDef->walk(visitor);
}

void ReturnStmt::walk(Visitor& visitor)
{
    visitor.visit(*this);
    expr->walk(visitor);
}

void LiteralExpr::walk(Visitor& visitor)
{
    visitor.visit(*this);
}

void TranslationUnit::walk(Visitor& visitor)
{
    visitor.visit(*this);
    foreach (QSharedPointer<AliasDecl> alias, aliasDecl)
        alias->walk(visitor);
    foreach (QSharedPointer<FuncDecl> func, funcDecl)
        func->walk(visitor);
}

void VarExpr::walk(Visitor& visitor)
{
    visitor.visit(*this);
}
