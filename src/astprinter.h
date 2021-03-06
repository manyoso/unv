#ifndef astprinter_h
#define astprinter_h

#include <QtCore>
#include "visitor.h"

class SourceBuffer;

class ASTPrinter : public Visitor {
public:
    ASTPrinter(SourceBuffer* source, QTextStream* stream);
    ~ASTPrinter();
    void walk();

    virtual void begin(Node&);
    virtual void end(Node&);

    virtual void visit(BinaryExpr&);
    virtual void visit(IncludeDecl&);
    virtual void visit(FuncCallExpr&);
    virtual void visit(FuncDecl&);
    virtual void visit(LiteralExpr&);
    virtual void visit(TypeCtorExpr&);
    virtual void visit(TypeDecl&);
    virtual void visit(TypeParam&);
    virtual void visit(TypeObject&);
    virtual void visit(VarExpr&);
    virtual void visit(VarDeclStmt&);

private:
    QString indent() const;
    int m_scope;
    QTextStream* m_stream;
    SourceBuffer* m_source;
};

#endif // astprinter_h
