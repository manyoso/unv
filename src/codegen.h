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

    /*!
     * \brief walks the AST and returns the generated LLVM IR
     * @return an LLVM IR representation of the AST in the form of a QString
     */
    QString generateLLVMIR();

private:
    virtual void begin(Node&) {}
    virtual void end(Node&) {}
    virtual void visit(FuncDecl&);
    void registerFuncDecl(FuncDecl*);
    void codegen(FuncDef* node, llvm::Type* type = 0);
    void codegen(Stmt* node, llvm::Type* type = 0);
    void codegen(IfStmt* node, llvm::Type* type = 0);
    void codegen(ReturnStmt* node, llvm::Type* type = 0);
    void codegen(VarDeclStmt* node, llvm::Type* type = 0);
    llvm::Value* codegen(BinaryExpr* node, llvm::Type* type = 0);
    llvm::Value* codegen(Expr* node, llvm::Type* type = 0);
    llvm::Value* codegen(FuncCallExpr* node, llvm::Type* type = 0);
    llvm::Value* codegen(LiteralExpr* node, llvm::Type* type = 0);
    llvm::Value* codegen(VarExpr* node, llvm::Type* type = 0);
    llvm::Type* toPrimitiveType(const QString&) const;
    void comparisonOfSigns(Token tok, bool lSigned, bool rSigned);

private:
    SourceBuffer* m_source;
    Context m_context;
    Module m_module;
    Builder m_builder;
    bool m_declPass;
    QHash<QString, llvm::Value*> m_namedValues;
};

#endif // codegen_h
