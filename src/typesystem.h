#ifndef typesystem_h
#define typesystem_h

#include <QtCore>

#include "token.h"

class SourceBuffer;
struct Expr;
struct TypeDecl;
struct FuncDecl;

namespace llvm {
class Type;
}
typedef llvm::Type TypeHandle;

struct TypeRef {
    virtual ~TypeRef() {}
    virtual QStringRef refName() const = 0;
    virtual QStringRef typeName() const = 0;
};

struct TypeInfo {
    TypeInfo() : handle(0) {}
    virtual ~TypeInfo() {}
    virtual QStringRef typeName() const = 0;

    virtual bool isNode() const { return false; }
    virtual bool isBuiltin() const { return false; }
    virtual bool isProduct() const { return false; }
    virtual bool isFunction() const { return false; }
    virtual bool isAlias() const { return false; }
    virtual bool isSignedInt() const { return false; }
    virtual QList<TypeRef*> typeRefList() const { return QList<TypeRef*>(); }
    virtual TypeRef* returnTypeRef() const { return 0; }

    TypeHandle* handle;
};

struct Builtin : public TypeInfo {
    virtual QStringRef typeName() const { return &_typeName; }
    virtual bool isBuiltin() const { return true; }
    virtual bool isSignedInt() const { return _isSignedInt; }

    QString _typeName;
    bool _isSignedInt;
};

class TypeSystem {
public:
    TypeSystem(SourceBuffer*);

    bool addType(TypeDecl&);
    bool addFunction(FuncDecl&);

    TypeInfo* toType(const QString& name) const;
    TypeInfo* toType(const QStringRef& name) const;
    TypeInfo* toTypeAndCheck(const Token& name) const;
    TypeInfo* typeInfoForExpr(Expr* node) const;
    void checkCompatibleTypes(Expr*, Expr*) const;

    void clearNamedTypes()
    { m_namedTypes.clear(); }
    void insertNamedType(const QString& name, TypeInfo* info)
    { m_namedTypes.insert(name, info); }

private:
    void addBuiltin(const QString& typeName, bool isSignedInt = false);

private:
    QHash<QString, QString> m_aliasHash;
    QHash<QString, TypeInfo*> m_typeHash;
    QList<QSharedPointer<Builtin> > m_builtins;
    SourceBuffer* m_source;
    QHash<QString, TypeInfo*> m_namedTypes;
};

#endif // typesystem_h
