#ifndef symbols_h
#define symbols_h

#include <QtCore>

class TypeDecl;
class FuncDecl;

class Symbols {
public:
    Symbols();
    ~Symbols();

private:
    QList<TypeDecl> m_typeList;
    QList<FuncDecl> m_funcList;
};

#endif // symbols_h
