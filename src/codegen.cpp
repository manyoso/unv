#include "codegen.h"
#include "options.h"
#include "sourcebuffer.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

#include <llvm/Analysis/Verifier.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>

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
}

void CodeGen::walk()
{
    Visitor::walk(m_source->translationUnit());

    QString outputFile = Options::instance()->outputFile();

    if (outputFile.isEmpty()) {
        llvm::raw_fd_ostream stream(STDOUT_FILENO, true /*shouldClose*/);
        m_module->print(stream, 0);
        stream.flush();
    } else {
        QFile f(outputFile);
        if (f.open(QIODevice::WriteOnly)) {
            llvm::raw_fd_ostream stream(f.handle(), false /*shouldClose*/);
            m_module->print(stream, 0);
            stream.flush();
            f.close();
        } else {
            m_source->error(Token(), QString("can not write to file $0").arg(outputFile), SourceBuffer::Fatal);
        }
    }
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

    int i = 0;
    for (llvm::Function::arg_iterator it = f->arg_begin(); it != f->arg_end(); ++it, ++i) {
        QSharedPointer<FuncDeclArg> arg = node.args.at(i);
        LLVMString name(m_source->textForToken(arg->name));
        it->setName(name);
    }

    llvm::BasicBlock *block = llvm::BasicBlock::Create(*m_context, "entry", f);
    m_builder->SetInsertPoint(block);

    if (llvm::Value* value = codegen(*node.funcDef))
        m_builder->CreateRet(value);
    else
        m_builder->CreateRetVoid();
    llvm::verifyFunction(*f);
}

llvm::Value* CodeGen::codegen(FuncDef&)
{
    return llvm::ConstantInt::get(*m_context, llvm::APInt(32, "0", 10));;
}

llvm::Value* CodeGen::codegen(LiteralExpr& node)
{
    if (node.literal.type == Digits) {
        LLVMString number(m_source->textForToken(node.literal));
        return llvm::ConstantInt::get(*m_context, llvm::APInt(64, number, 10));
    }
    return 0;
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
