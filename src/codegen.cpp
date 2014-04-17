#include "codegen.h"
#include "options.h"
#include "sourcebuffer.h"

#include <limits>

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
    , m_declPass(true)
{
}

CodeGen::~CodeGen()
{
}

QString CodeGen::generateLLVMIR()
{
    // Walk the tree for the first pass to register all declarations
    Visitor::walk(m_source->translationUnit());
    m_declPass = false;
    // Walk the tree for the second pass to generate the rest of the code
    Visitor::walk(m_source->translationUnit());

    std::string str;
    llvm::raw_string_ostream stream(str);
    m_module->print(stream, 0);
    stream.flush();
    return QString::fromStdString(str);
}

void CodeGen::visit(FuncDecl& node)
{
    if (m_declPass) {
        registerFuncDecl(&node);
        return;
    }

    LLVMString name(m_source->textForToken(node.name));
    llvm::Function *f = m_module->getFunction(name);

    int i = 0;
    m_namedValues.clear();
    for (llvm::Function::arg_iterator it = f->arg_begin(); it != f->arg_end(); ++it, ++i) {
        QSharedPointer<FuncDeclArg> arg = node.args.at(i);
        m_namedValues.insert(m_source->textForToken(arg->name), it);
    }

    llvm::BasicBlock *block = llvm::BasicBlock::Create(*m_context, "entry", f);
    m_builder->SetInsertPoint(block);
    codegen(node.funcDef.data());
    llvm::verifyFunction(*f);
}

void CodeGen::registerFuncDecl(FuncDecl* node) {
    LLVMString name(m_source->textForToken(node->name));
    QList<llvm::Type*> params;
    foreach (QSharedPointer<FuncDeclArg> arg, node->args) {
        QString type = m_source->symbols().toTypeAndCheck(arg->type);
        params.append(toPrimitiveType(type));
    }
    llvm::Type* returnType = toPrimitiveType(m_source->symbols().toTypeAndCheck(node->returnType));
    llvm::FunctionType *ft = llvm::FunctionType::get(returnType, params.toVector().toStdVector(), false /*varargs*/);
    llvm::Function *f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, name, m_module.data());
    Q_ASSERT(f->getName() == name);

    int i = 0;
    for (llvm::Function::arg_iterator it = f->arg_begin(); it != f->arg_end(); ++it, ++i) {
        QSharedPointer<FuncDeclArg> arg = node->args.at(i);
        LLVMString name(m_source->textForToken(arg->name));
        it->setName(name);
    }
}

void CodeGen::codegen(FuncDef* node, llvm::Type*)
{
    foreach (QSharedPointer<Stmt> stmt, node->stmts)
        codegen(stmt.data());
}

void CodeGen::codegen(Stmt* node, llvm::Type*)
{
    switch (node->kind) {
    case Node::_IfStmt:
        codegen(static_cast<IfStmt*>(node));
        break;
    case Node::_ReturnStmt:
        codegen(static_cast<ReturnStmt*>(node));
        break;
    case Node::_VarDeclStmt:
        codegen(static_cast<VarDeclStmt*>(node));
        break;
    default:
        Q_ASSERT(false); // should not be reached
        return;
    }
}

void CodeGen::codegen(IfStmt* node, llvm::Type*)
{
    llvm::Value *condition = codegen(node->expr.data());
    Q_ASSERT(condition);
    if (!condition)
        return;

    llvm::Function* f = m_builder->GetInsertBlock()->getParent();
    Q_ASSERT(f);
    if (!f)
        return;

    llvm::BasicBlock* then = llvm::BasicBlock::Create(*m_context, "then", f);
    llvm::BasicBlock* ifcont = llvm::BasicBlock::Create(*m_context, "ifcont");

    m_builder->CreateCondBr(condition, then, ifcont);

    m_builder->SetInsertPoint(then);

    codegen(node->stmt.data());

    f->getBasicBlockList().push_back(ifcont);
    m_builder->SetInsertPoint(ifcont);
}

void CodeGen::codegen(ReturnStmt* node, llvm::Type*)
{
    llvm::Function* f = m_builder->GetInsertBlock()->getParent();

    Q_ASSERT(f);
    if (!f)
        return;

    if (llvm::Value* value = codegen(node->expr.data(), f->getReturnType())) {
        m_builder->CreateRet(value);
        return;
    }

    m_source->error(node->keyword, "return statement of void is not allowed", SourceBuffer::Fatal);
}

void CodeGen::codegen(VarDeclStmt* node, llvm::Type*)
{
    llvm::Function* f = m_builder->GetInsertBlock()->getParent();

    Q_ASSERT(f);
    if (!f)
        return;

    QString typeName = m_source->symbols().toTypeAndCheck(node->type);
    QString name = m_source->textForToken(node->name);

    llvm::Type* type = toPrimitiveType(typeName);
    Q_ASSERT(type);
    if (!type)
        return;

    llvm::Value* value = codegen(node->expr.data(), type);
    Q_ASSERT(value);
    if (!value)
        return;

    m_namedValues.insert(name, value);
}

llvm::Value* CodeGen::codegen(BinaryExpr* node, llvm::Type*)
{
    llvm::Value *l = 0;
    llvm::Value *r = 0;
    if (node->lhs->kind != Node::_LiteralExpr)
        l = codegen(node->lhs.data());
    if (node->rhs->kind != Node::_LiteralExpr)
        r = codegen(node->rhs.data());

    if (!l && !r) {
        m_source->error(static_cast<LiteralExpr*>(node->lhs.data())->literal,
            "we do not support binary expressions involving two literals yet",
            SourceBuffer::Fatal);
        return 0;
    }

    if (!l)
        l = codegen(node->lhs.data(), r->getType());
    if (!r)
        r = codegen(node->rhs.data(), l->getType());

    Q_ASSERT(l && r);
    if (!l && !r) return 0;

    switch (node->op) {
    case BinaryExpr::Equality:
        return m_builder->CreateICmpEQ(l, r, "equaltmp");
    case BinaryExpr::LessThanOrEquality:
    case BinaryExpr::GreaterThanOrEquality:
        Q_ASSERT(false); // should not be reached
        return 0;
    case BinaryExpr::Addition:
        return m_builder->CreateAdd(l, r, "addtmp");
    case BinaryExpr::Subtraction:
        return m_builder->CreateSub(l, r, "subtmp");
    case BinaryExpr::Multiplication:
        Q_ASSERT(false); // should not be reached
        return 0;
    }
}

llvm::Value* CodeGen::codegen(Expr* node, llvm::Type* type)
{
    switch (node->kind) {
    case Node::_BinaryExpr:
        return codegen(static_cast<BinaryExpr*>(node), type);
    case Node::_FuncCallExpr:
        return codegen(static_cast<FuncCallExpr*>(node), type);
    case Node::_LiteralExpr:
        return codegen(static_cast<LiteralExpr*>(node), type);
    case Node::_VarExpr:
        return codegen(static_cast<VarExpr*>(node), type);
    default:
        Q_ASSERT(false); // should not be reached
        return 0;
    }
}

llvm::Value* CodeGen::codegen(FuncCallExpr* node, llvm::Type*)
{
    LLVMString callee = m_source->textForToken(node->callee);
    llvm::Function *calleeFunction = m_module->getFunction(callee);
    if (!calleeFunction) {
        m_source->error(node->callee, "unknown function reference", SourceBuffer::Fatal);
        return 0;
    }

    if (int(calleeFunction->arg_size()) != node->args.size()) {
        m_source->error(node->callee, "incorrect number of arguments passed", SourceBuffer::Fatal);
        return 0;
    }

    int i = 0;
    QList<llvm::Value*> args;
    for (llvm::Function::arg_iterator it = calleeFunction->arg_begin();
            it != calleeFunction->arg_end();
            ++it, ++i) {
        QSharedPointer<Expr> arg = node->args.at(i);
        llvm::Value* value = codegen(arg.data(), it->getType());
        Q_ASSERT(value);
        if (!value)
            return 0;
        args.append(value);
    }

    return m_builder->CreateCall(calleeFunction, args.toVector().toStdVector(), "calltmp");
}

llvm::Value* CodeGen::codegen(LiteralExpr* node, llvm::Type* type)
{
    Q_ASSERT(type);
    if (!type)
        return 0;

    if (node->literal.type == Digits) {
        if (!type->isIntegerTy()) {
            m_source->error(node->literal, "non-integer literal not supported yet", SourceBuffer::Fatal);
            return 0;
        }

        llvm::IntegerType* integerType = static_cast<llvm::IntegerType*>(type);
        QString digits = m_source->textForToken(node->literal);
        unsigned numbits = integerType->getBitWidth();
        bool isSigned = integerType->getSignBit();
        bool success = false;

        if (!isSigned) {
            quint64 n = digits.toULongLong(&success);

            bool outOfRange = false;
            if ((numbits == 1 && n > 1)
                || (numbits == 8 && n > 255)
                || (numbits == 16 && n > std::numeric_limits<uint16_t>::max())
                || (numbits == 32 && n > std::numeric_limits<uint32_t>::max())
                || (numbits == 64 && n > std::numeric_limits<uint64_t>::max()))
                outOfRange = true;

            if (!success || outOfRange) {
                m_source->error(node->literal, "unsigned integer literal out of range", SourceBuffer::Fatal);
                return 0;
            }

            return llvm::ConstantInt::get(*m_context, llvm::APInt(numbits, n, false));
        } else {
            qint64 n = digits.toLongLong(&success);

            bool outOfRange = false;
            if ((numbits == 1 && (n < 0 || n > 1))
                || (numbits == 8 && (n < -128 || n > 127))
                || (numbits == 16 && (n < std::numeric_limits<int16_t>::min() || n > std::numeric_limits<int16_t>::max()))
                || (numbits == 32 && (n < std::numeric_limits<int32_t>::min() || n > std::numeric_limits<int32_t>::max()))
                || (numbits == 64 && (n < std::numeric_limits<int64_t>::min() || n > std::numeric_limits<int64_t>::max())))
                outOfRange = true;

            if (!success || outOfRange) {
                m_source->error(node->literal, "signed integer literal out of range", SourceBuffer::Fatal);
                return 0;
            }

            return llvm::ConstantInt::get(*m_context, llvm::APInt(numbits, n, true));
        }
    }
    Q_ASSERT(false); // should not be reached
    return 0;
}

llvm::Value* CodeGen::codegen(VarExpr* node, llvm::Type* type)
{
    QString name = m_source->textForToken(node->var);
    llvm::Value *value = m_namedValues.contains(name) ? m_namedValues.value(name) : 0;
    if (!value)
        m_source->error(node->var, "unknown variable name", SourceBuffer::Fatal);
    if (type && value->getType() != type)
        m_source->error(node->var, "unknown variable type", SourceBuffer::Fatal);
    return value;
}

llvm::Type* CodeGen::toPrimitiveType(const QString& string) const
{
    if (string.isEmpty())
        return llvm::Type::getVoidTy(*m_context);
    else if (string == "_builtin_bit_")
        return llvm::Type::getInt1Ty(*m_context);
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
