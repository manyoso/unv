#include "symbols.h"
#include "sourcebuffer.h"

Symbols::Symbols(SourceBuffer* source)
    : m_source(source)
{
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

bool Symbols::addAlias(AliasDecl& decl)
{
    QString alias = m_source->textForToken(decl.alias);

    if (m_aliasHash.contains(alias)) {
        m_source->error(decl.alias, "alias declaration previously declared");
        return false;
    }

    QString type = m_source->textForToken(decl.type);
    m_aliasHash.insert(alias, type);
    return true;
}

bool Symbols::addType(TypeDecl& decl)
{
    QString symbol(m_source->textForToken(decl.type));

    if (m_typeList.contains(symbol)) {
        m_source->error(decl.type, "type declaration previously declared");
        return false;
    }

    m_typeList.append(symbol);
    return true;
}

bool Symbols::addFunction(FuncDecl& decl)
{
    QString name(m_source->textForToken(decl.name));
    QString returnType(m_source->textForToken(decl.returnType));

    QStringList args;
    foreach (QSharedPointer<FuncDeclArg> arg, decl.args)
        args.append(m_source->textForToken(arg->type));

    QString symbol = name + args.join("") + returnType;

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
