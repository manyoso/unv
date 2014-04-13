#ifndef codegen_h
#define codegen_h

#include <QtCore>
#include "visitor.h"

class SourceBuffer;

namespace llvm {
    class ConstantFolder;
    template<bool preserveNames>
    class IRBuilderDefaultInserter;
    class LLVMContext;
    template<bool preserveNames, typename T, typename Inserter>
    class IRBuilder;
    class Module;
    typedef llvm::IRBuilder<true, llvm::ConstantFolder, llvm::IRBuilderDefaultInserter<true> > Builder;
    class Type;
}

typedef QSharedPointer<llvm::LLVMContext> Context;
typedef QSharedPointer<llvm::Module> Module;
typedef QSharedPointer<llvm::Builder> Builder;

class CodeGen : public Visitor {
public:
    CodeGen(SourceBuffer* source);
    ~CodeGen();
    void walk();

    virtual void begin(Node&);
    virtual void end(Node&);

    virtual void visit(AliasDecl&);
    virtual void visit(BinaryExpr&);
    virtual void visit(IfStmt&);
    virtual void visit(FuncCallExpr&);
    virtual void visit(FuncDef&);
    virtual void visit(FuncDeclArg&);
    virtual void visit(FuncDecl&);
    virtual void visit(LiteralExpr&);
    virtual void visit(ReturnStmt&);
    virtual void visit(TranslationUnit&);
    virtual void visit(TypeDecl&);
    virtual void visit(VarExpr&);

private:
    llvm::Type* toPrimitiveType(const QString&) const;

private:
    SourceBuffer* m_source;
    Context m_context;
    Module m_module;
    Builder m_builder;
};

#endif // codegen_h
