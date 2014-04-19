#include "symbols.h"
#include "sourcebuffer.h"

Symbols::Symbols(SourceBuffer* source)
    : m_source(source)
{
    // 1 bit int types
    m_typeList.append("_builtin_bit_");

    // 8 bit int types
    m_typeList.append("_builtin_uint8_");
    m_typeList.append("_builtin_int8_");

    // 16 bit int types
    m_typeList.append("_builtin_uint16_");
    m_typeList.append("_builtin_int16_");

    // 32 bit int types
    m_typeList.append("_builtin_uint32_");
    m_typeList.append("_builtin_int32_");

    // 64 bit int types
    m_typeList.append("_builtin_uint64_");
    m_typeList.append("_builtin_int64_");
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

    if (m_typeList.contains(symbol)) {
        m_source->error(decl.name, "type declaration previously declared");
        return false;
    }

    m_typeList.append(symbol);
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

    if (m_funcList.contains(symbol)) {
        m_source->error(decl.name, "function declaration previously declared");
        return false;
    }

    m_funcList.append(symbol);
    return true;
}

QString Symbols::toTypeAndCheck(const Token& token) const
{
    QString type = m_source->textForToken(token);
    if (m_aliasHash.contains(type))
        type = m_aliasHash.value(type);
    if (!m_typeList.contains(type)) {
        m_source->error(token, "type has not been declared", SourceBuffer::Fatal);
        return QString();
    }
    return type;
}
