#include "typesystem.h"
#include "sourcebuffer.h"

TypeSystem::TypeSystem(SourceBuffer* source)
    : m_source(source)
{
    // 1 bit integer types
    addBuiltin("_builtin_bit_");

    // 8 bit integer types
    addBuiltin("_builtin_uint8_");
    addBuiltin("_builtin_int8_");

    // 16 bit integer types
    addBuiltin("_builtin_uint16_");
    addBuiltin("_builtin_int16_");

    // 32 bit integer types
    addBuiltin("_builtin_uint32_");
    addBuiltin("_builtin_int32_");

    // 64 bit integer types
    addBuiltin("_builtin_uint64_");
    addBuiltin("_builtin_int64_");

    // 32-bit floating point type
    addBuiltin("_builtin_float_");

    // 64-bit floating point type
    addBuiltin("_builtin_double_");
}

void TypeSystem::addBuiltin(const QString& name)
{
    TypeInfo* info = new TypeInfo(name, TypeInfo::Builtin);
    m_typeHash.insert(name, QSharedPointer<TypeInfo>(info));
}

bool TypeSystem::addType(TypeDecl& decl)
{
    QString name(m_source->textForToken(decl.name));

    QStringList objects;
    foreach (QSharedPointer<TypeObject> object, decl.objects)
        objects.append(m_source->textForToken(object->type));

    if (m_typeHash.contains(name)) {
        m_source->error(decl.name, "type declaration previously declared");
        return false;
    }

    if (objects.size() < 1) {
        m_source->error(decl.name, "type declaration must declare a type");
        return false;
    } else if (objects.size() < 2) {
        QString alias = name;
        if (m_aliasHash.contains(alias)) {
            m_source->error(decl.name, "alias for name previously declared");
            return false;
        }

        QString type = objects.first();
        m_aliasHash.insert(alias, type);

        TypeInfo* aliasType = m_typeHash.value(type).data();
        Q_ASSERT(aliasType);

        TypeInfo* info = new TypeInfo(name, aliasType->kind);
        info->isAlias = true;
        m_typeHash.insert(name, QSharedPointer<TypeInfo>(info));
        return true;
    }

    TypeInfo* info = new TypeInfo(name, TypeInfo::Product);
    m_typeHash.insert(name, QSharedPointer<TypeInfo>(info));
    return true;
}

bool TypeSystem::addFunction(FuncDecl& decl)
{
    QString name(m_source->textForToken(decl.name));

    if (m_typeHash.contains(name)) {
        m_source->error(decl.name, "function declaration previously declared");
        return false;
    }

    TypeInfo* info = new TypeInfo(name, TypeInfo::Function);
    m_typeHash.insert(name, QSharedPointer<TypeInfo>(info));
    return true;
}

TypeInfo* TypeSystem::toType(const QString& name) const
{
    return m_typeHash.value(name).data();
}

TypeInfo* TypeSystem::toTypeAndCheck(const Token& name) const
{
    QString type = m_source->textForToken(name);
    if (m_aliasHash.contains(type))
        type = m_aliasHash.value(type);
    if (!m_typeHash.contains(type)) {
        m_source->error(name, "type has not been declared", SourceBuffer::Fatal);
        return 0;
    }
    return m_typeHash.value(type).data();
}
