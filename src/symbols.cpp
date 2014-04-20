#include "symbols.h"
#include "sourcebuffer.h"

Symbols::Symbols(SourceBuffer* source)
    : m_source(source)
{
    // 1 bit integer types
    m_typeHash.insert("_builtin_bit_", "_builtin_bit_");

    // 8 bit integer types
    m_typeHash.insert("_builtin_uint8_", "_builtin_uint8_");
    m_typeHash.insert("_builtin_int8_", "_builtin_int8_");

    // 16 bit integer types
    m_typeHash.insert("_builtin_uint16_", "_builtin_uint16_");
    m_typeHash.insert("_builtin_int16_", "_builtin_int16_");

    // 32 bit integer types
    m_typeHash.insert("_builtin_uint32_", "_builtin_uint32_");
    m_typeHash.insert("_builtin_int32_", "_builtin_int32_");

    // 64 bit integer types
    m_typeHash.insert("_builtin_uint64_", "_builtin_uint64_");
    m_typeHash.insert("_builtin_int64_", "_builtin_int64_");

    // 32-bit floating point type
    m_typeHash.insert("_builtin_float_", "_builtin_float_");

    // 64-bit floating point type
    m_typeHash.insert("_builtin_double_", "_builtin_double_");
}

bool Symbols::addType(TypeDecl& decl)
{
    QString name(m_source->textForToken(decl.name));

    QStringList objects;
    foreach (QSharedPointer<TypeObject> object, decl.objects)
        objects.append(m_source->textForToken(object->type));

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
    }

    QString symbol = name + objects.join("");

    if (m_typeHash.contains(name)) {
        m_source->error(decl.name, "type declaration previously declared");
        return false;
    }

    m_typeHash.insert(name, symbol);
    return true;
}

bool Symbols::addFunction(FuncDecl& decl)
{
    QString name(m_source->textForToken(decl.name));
    QString returnType(m_source->textForToken(decl.returnType));

    QStringList objects;
    foreach (QSharedPointer<TypeObject> object, decl.objects)
        objects.append(m_source->textForToken(object->type));

    QString symbol = name + objects.join("") + returnType;

    if (m_funcHash.contains(name)) {
        m_source->error(decl.name, "function declaration previously declared");
        return false;
    }

    m_funcHash.insert(name, symbol);
    return true;
}

QString Symbols::toTypeAndCheck(const Token& token) const
{
    QString type = m_source->textForToken(token);
    if (m_aliasHash.contains(type))
        type = m_aliasHash.value(type);
    if (!m_typeHash.contains(type)) {
        m_source->error(token, "type has not been declared", SourceBuffer::Fatal);
        return QString();
    }
    return type;
}
