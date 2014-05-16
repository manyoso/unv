#include "ast.h"
#include "visitor.h"

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

void IncludeDecl::walk(Visitor& visitor)
{
    visitor.begin(*this);
    visitor.visit(*this);
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

void FuncDecl::walk(Visitor& visitor)
{
    visitor.begin(*this);
    visitor.visit(*this);
    foreach (QSharedPointer<TypeParam> param, params)
        param->walk(visitor);
    foreach (QSharedPointer<TypeObject> obj, objects)
        obj->walk(visitor);
    returnType->walk(visitor);
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
    foreach (QSharedPointer<IncludeDecl> include, includeDecl)
        include->walk(visitor);
    foreach (QSharedPointer<TypeDecl> type, typeDecl)
        type->walk(visitor);
    foreach (QSharedPointer<FuncDecl> func, funcDecl)
        func->walk(visitor);
    visitor.end(*this);
}

void TypeCtorExpr::walk(Visitor& visitor)
{
    visitor.begin(*this);
    visitor.visit(*this);
    foreach (QSharedPointer<Expr> arg, args)
        arg->walk(visitor);
    visitor.end(*this);
}

void TypeDecl::walk(Visitor& visitor)
{
    visitor.begin(*this);
    visitor.visit(*this);
    foreach (QSharedPointer<TypeParam> param, params)
        param->walk(visitor);
    foreach (QSharedPointer<TypeObject> obj, objects)
        obj->walk(visitor);
    visitor.end(*this);
}

void TypeObject::walk(Visitor& visitor)
{
    visitor.begin(*this);
    visitor.visit(*this);
    visitor.end(*this);
}

void TypeParam::walk(Visitor& visitor)
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