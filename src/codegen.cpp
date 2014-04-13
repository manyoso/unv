#include "codegen.h"
#include "sourcebuffer.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#pragma clang diagnostic pop

class LLVMString {
public:
    LLVMString(const QString& string) : m_string(string.toLatin1()) { }

    llvm::StringRef toStringRef() const
    {
        return llvm::StringRef(m_string.constData(), m_string.size());
    }

    operator llvm::StringRef() const
    {
        return toStringRef();
    }

    operator llvm::Twine() const
    {
        return llvm::Twine(m_string.constData());
    }

private:
    QByteArray m_string;
};

CodeGen::CodeGen(SourceBuffer* buffer)
    : m_source(buffer)
    , m_context(new llvm::LLVMContext)
    , m_module(new llvm::Module(LLVMString(buffer->module()), (*m_context)))
    , m_builder(new llvm::Builder(*m_context))
{
}

CodeGen::~CodeGen()
{
    m_module->dump();
}

void CodeGen::walk()
{
    Visitor::walk(m_source->translationUnit());
}

void CodeGen::begin(Node&)
{
}

void CodeGen::end(Node&)
{
}

void CodeGen::visit(AliasDecl&)
{
}

void CodeGen::visit(BinaryExpr&)
{
}

void CodeGen::visit(IfStmt&)
{
}

void CodeGen::visit(FuncCallExpr&)
{
}

void CodeGen::visit(FuncDef&)
{
}

void CodeGen::visit(FuncDeclArg&)
{
}

void CodeGen::visit(FuncDecl& node)
{
    LLVMString name(m_source->textForToken(node.name));
    QList<llvm::Type*> params;
    foreach (QSharedPointer<FuncDeclArg> arg, node.args) {
        QString type = m_source->symbols().toTypeAndCheck(arg->type);
        params.append(toPrimitiveType(type));
    }
    llvm::Type* returnType = toPrimitiveType(m_source->symbols().toTypeAndCheck(node.returnType));
    llvm::FunctionType *ft = llvm::FunctionType::get(returnType, params.toVector().toStdVector(), false /*varargs*/);
    llvm::Function *f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, name, m_module.data());
    Q_ASSERT(f->getName() == name);
}

void CodeGen::visit(LiteralExpr&)
{
}

void CodeGen::visit(ReturnStmt&)
{
}

void CodeGen::visit(TranslationUnit&)
{
}

void CodeGen::visit(TypeDecl&)
{
}

void CodeGen::visit(VarExpr&)
{
}

llvm::Type* CodeGen::toPrimitiveType(const QString& string) const
{
    if (string.isEmpty())
        return llvm::Type::getVoidTy(*m_context);
//    else if (string == "_builtin_uint8_")
    else if (string == "_builtin_int8_")
        return llvm::Type::getInt8Ty(*m_context);
//    else if (string == "_builtin_uint16_")
    else if (string == "_builtin_int16_")
        return llvm::Type::getInt16Ty(*m_context);
//    else if (string == "_builtin_uint32_")
    else if (string == "_builtin_int32_")
        return llvm::Type::getInt32Ty(*m_context);
//    else if (string == "_builtin_uint64_")
    else if (string == "_builtin_int64_")
        return llvm::Type::getInt64Ty(*m_context);
    Q_ASSERT(false);
    return llvm::Type::getVoidTy(*m_context);
}
