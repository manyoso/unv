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
    class Function;
    class Value;
}

typedef QSharedPointer<llvm::LLVMContext> Context;
typedef QSharedPointer<llvm::Module> Module;
typedef QSharedPointer<llvm::Builder> Builder;

class CodeGen : public Visitor {
public:
    CodeGen(SourceBuffer* source);
    ~CodeGen();
    void walk();

private:
    virtual void begin(Node&) {}
    virtual void end(Node&) {}
    virtual void visit(FuncDecl&);
    llvm::Value* codegen(FuncDef& node);
    llvm::Value* codegen(LiteralExpr& node);
    llvm::Type* toPrimitiveType(const QString&) const;

private:
    SourceBuffer* m_source;
    Context m_context;
    Module m_module;
    Builder m_builder;
};

#endif // codegen_h
