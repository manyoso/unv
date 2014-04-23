#include "typesystem.h"
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
