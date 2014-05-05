#include "typesystem.h"
#include "ast.h"
#include "sourcebuffer.h"

TypeSystem::TypeSystem(SourceBuffer* source)
    : m_source(source)
{
    // 1 bit integer types
    addBuiltin("_builtin_bit_");

    // 8 bit integer types
    addBuiltin("_builtin_uint8_");
    addBuiltin("_builtin_int8_", true /*signedInt*/);

    // 16 bit integer types
    addBuiltin("_builtin_uint16_");
    addBuiltin("_builtin_int16_", true /*signedInt*/);

    // 32 bit integer types
    addBuiltin("_builtin_uint32_");
    addBuiltin("_builtin_int32_", true /*signedInt*/);

    // 64 bit integer types
    addBuiltin("_builtin_uint64_");
    addBuiltin("_builtin_int64_", true /*signedInt*/);

    // 32-bit floating point type
    addBuiltin("_builtin_float_");

    // 64-bit floating point type
    addBuiltin("_builtin_double_");

    // 8 bit constant array
    addBuiltin("_builtin_uint8_array_");
}

void TypeSystem::addBuiltin(const QString& typeName, bool isSignedInt)
{
    Builtin* info = new Builtin;
    info->_typeName = typeName;
    info->_isSignedInt = isSignedInt;
    m_typeHash.insert(typeName, info);
    m_builtins.append(QSharedPointer<Builtin>(info));
}

bool TypeSystem::addType(TypeDecl& decl)
{
    QString name = decl.name.toString();
    if (m_typeHash.contains(name)) {
        m_source->error(decl.name, "type declaration previously declared");
        return false;
    }

    if (decl.objects.size() < 1) {
        m_source->error(decl.name, "type declaration must declare a type");
        return false;
    } else if (decl.objects.size() < 2) {
        QString alias = name;
        if (m_aliasHash.contains(alias)) {
            m_source->error(decl.name, "alias for name previously declared");
            return false;
        }

        QSharedPointer<TypeObject> object = decl.objects.first();
        QString type = object->type.toString();
        m_aliasHash.insert(alias, type);

        m_typeHash.insert(alias, &decl);
        return true;
    }

    m_typeHash.insert(name, &decl);
    return true;
}

bool TypeSystem::addFunction(FuncDecl& decl)
{
    QString name = decl.name.toString();

    if (m_typeHash.contains(name)) {
        m_source->error(decl.name, "function declaration previously declared");
        return false;
    }

    m_typeHash.insert(name, &decl);
    return true;
}

TypeInfo* TypeSystem::toType(const QStringRef& name) const
{
    return toType(name.toString());
}

TypeInfo* TypeSystem::toType(const QString& name) const
{
    return m_typeHash.value(name);
}

TypeInfo* TypeSystem::toTypeAndCheck(const Token& tok) const
{
    QString type = tok.toString();
    if (m_aliasHash.contains(type))
        type = m_aliasHash.value(type);
    if (!m_typeHash.contains(type)) {
        m_source->error(tok, "type has not been declared", SourceBuffer::Fatal);
        return 0;
    }
    return m_typeHash.value(type);
}

TypeInfo* TypeSystem::typeInfoForExpr(Expr* node) const
{
    switch (node->kind) {
    case Node::_BinaryExpr:
    {
        BinaryExpr* expr = static_cast<BinaryExpr*>(node);

        if (expr->lhs->kind == Node::_LiteralExpr && expr->rhs->kind == Node::_LiteralExpr) {
            m_source->error(expr->lhs->start, "both sides of binary expression are literal expressions", SourceBuffer::Fatal);
            return 0;
        }

        if (TypeInfo* info = typeInfoForExpr(expr->lhs.data()))
            return info;

        if (TypeInfo* info = typeInfoForExpr(expr->rhs.data()))
            return info;

        m_source->error(expr->lhs->start, "can not determine type for binary expression", SourceBuffer::Fatal);
        return 0;
    }
    case Node::_FuncCallExpr:
    {
        FuncCallExpr* expr = static_cast<FuncCallExpr*>(node);
        TypeInfo* function = toTypeAndCheck(expr->callee);
        if (TypeRef* returnTypeRef = function->returnTypeRef())
            if (TypeInfo* returnType = toType(returnTypeRef->typeName()))
                return returnType;

        m_source->error(expr->callee, "can not determine type for function call expression", SourceBuffer::Fatal);
        return 0;
    }
    case Node::_LiteralExpr:
        return 0;
    case Node::_VarExpr:
    {
        VarExpr* expr = static_cast<VarExpr*>(node);
        QString name = expr->var.toString();
        if (m_namedTypes.contains(name))
            return m_namedTypes.value(name);

        m_source->error(expr->var, "can not determine type for variable expression", SourceBuffer::Fatal);
        return 0;
    }
    case Node::_TypeCtorExpr:
    {
        TypeCtorExpr* expr = static_cast<TypeCtorExpr*>(node);
        if (expr->type.type == Undefined)
            return typeInfoForExpr(expr->args.first().data());

        if (TypeInfo* info = toTypeAndCheck(expr->type))
            return info;

        m_source->error(expr->type, "can not determine type for type ctor expression", SourceBuffer::Fatal);
        return 0;
    }
    default:
        assert(false); // should not be reached
        return 0;
    }
}

void TypeSystem::checkCompatibleTypes(Expr* expr1, Expr* expr2) const
{
    assert(expr1);
    assert(expr2);

    TypeInfo* infoForExpr1 = typeInfoForExpr(expr1);
    TypeInfo* infoForExpr2 = typeInfoForExpr(expr2);

    if (!infoForExpr1 || !infoForExpr2)
        return;

    if (infoForExpr1->isBuiltin() != infoForExpr2->isBuiltin()
        || infoForExpr1->isProduct() != infoForExpr2->isProduct()
        || infoForExpr1->isFunction() != infoForExpr2->isFunction()
        || infoForExpr1->isAlias() != infoForExpr2->isAlias())
        m_source->error(expr1->start, "type incompatibility for binary expression operands", SourceBuffer::Fatal);

    if (infoForExpr1->isSignedInt() != infoForExpr2->isSignedInt())
        m_source->error(expr1->start, "comparison of signed and unsigned integers not supported", SourceBuffer::Fatal);
}
