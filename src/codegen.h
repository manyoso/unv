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

struct TypeInfo;

class CodeGen : public Visitor {
public:
    CodeGen(SourceBuffer* source);
    CodeGen(SourceBuffer* source, Context, Module);
    ~CodeGen();

    /*!
     * \brief walks the AST and returns the generated LLVM IR
     * @return an LLVM IR representation of the AST in the form of a QString
     */
    QString generateLLVMIR();

private:
    virtual void begin(Node&) {}
    virtual void end(Node&) {}
    virtual void visit(IncludeDecl&);
    virtual void visit(TypeDecl&);
    virtual void visit(FuncDecl&);
    void registerBuiltins();
    void registerTypeDecl(TypeDecl*);
    void registerFuncDecl(FuncDecl*);
    void codegen(FuncDef* node);
    void codegen(Stmt* node);
    void codegen(IfStmt* node);
    void codegen(ReturnStmt* node);
    void codegen(VarDeclStmt* node);
    llvm::Value* codegen(BinaryExpr* node, TypeInfo* info);
    llvm::Value* codegen(Expr* node, TypeInfo* info);
    llvm::Value* codegen(FuncCallExpr* node, TypeInfo* info);
    llvm::Value* codegen(LiteralExpr* node, TypeInfo* info);
    llvm::Value* codegen(TypeCtorExpr* node, TypeInfo* info);
    llvm::Value* codegen(VarExpr* node, TypeInfo* info);
    llvm::Type* toCodeGenType(const Token& tok) const;

private:
    SourceBuffer* m_source;
    Context m_context;
    Module m_module;
    Builder m_builder;
    bool m_declPass;
    QHash<QString, llvm::Value*> m_namedValues;
};

#endif // codegen_h
