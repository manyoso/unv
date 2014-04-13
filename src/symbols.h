#ifndef symbols_h
#define symbols_h

#include <QtCore>

class SourceBuffer;
struct AliasDecl;
struct TypeDecl;
struct FuncDecl;

class Symbols {
public:
    Symbols(SourceBuffer*);

    bool addAlias(AliasDecl&);
    bool addType(TypeDecl&);
    bool addFunction(FuncDecl&);

private:
    QHash<QString, QString> m_aliasHash;
    QList<QString> m_typeList;
    QList<QString> m_funcList;
    SourceBuffer* m_source;
};

#endif // symbols_h
