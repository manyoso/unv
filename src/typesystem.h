#ifndef typesystem_h
#define typesystem_h

#include <QtCore>

#include "token.h"

class SourceBuffer;
struct TypeDecl;
struct FuncDecl;

namespace llvm {
class Type;
}
typedef llvm::Type TypeHandle;

struct TypeInfo {
    enum Kind {
        Builtin,
        Product,
        Function
    };

    TypeInfo(const QString& name, Kind kind)
        : name(name)
        , kind(kind)
        , handle(0)
        , isAlias(false)
        , isSignedInt(false)
    {}

    QString name;
    Kind kind;
    TypeHandle* handle;
    bool isAlias;
    bool isSignedInt;
};

class TypeSystem {
public:
    TypeSystem(SourceBuffer*);

    bool addType(TypeDecl&);
    bool addFunction(FuncDecl&);

    TypeInfo* toType(const QString &name) const;
    TypeInfo* toTypeAndCheck(const Token &name) const;

private:
    void addBuiltin(const QString& builtin);

private:
    QHash<QString, QString> m_aliasHash;
    QHash<QString, QSharedPointer<TypeInfo> > m_typeHash;
    SourceBuffer* m_source;
};

#endif // typesystem_h
