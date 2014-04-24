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
    LLVMString(const QStringRef& string) : m_string(string.toLatin1()) { }

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

static QString toQString(llvm::StringRef ref)
{
    return QString::fromLocal8Bit(ref.data(), ref.size());
}

CodeGen::CodeGen(SourceBuffer* buffer)
    : m_source(buffer)
    , m_context(new llvm::LLVMContext)
    , m_module(new llvm::Module(LLVMString(buffer->module()), (*m_context)))
    , m_builder(new llvm::Builder(*m_context))
    , m_declPass(true)
{
    TypeInfo* info = m_source->typeSystem().toType("_builtin_bit_");
    info->handle = llvm::Type::getInt1Ty(*m_context);

    info = m_source->typeSystem().toType("_builtin_uint8_");
    info->handle = llvm::Type::getInt8Ty(*m_context);

    info = m_source->typeSystem().toType("_builtin_int8_");
    info->handle = llvm::Type::getInt8Ty(*m_context);

    info = m_source->typeSystem().toType("_builtin_uint16_");
    info->handle = llvm::Type::getInt16Ty(*m_context);

    info = m_source->typeSystem().toType("_builtin_int16_");
    info->handle = llvm::Type::getInt16Ty(*m_context);

    info = m_source->typeSystem().toType("_builtin_uint32_");
    info->handle = llvm::Type::getInt32Ty(*m_context);

    info = m_source->typeSystem().toType("_builtin_int32_");
    info->handle = llvm::Type::getInt32Ty(*m_context);

    info = m_source->typeSystem().toType("_builtin_uint64_");
    info->handle = llvm::Type::getInt64Ty(*m_context);

    info = m_source->typeSystem().toType("_builtin_int64_");
    info->handle = llvm::Type::getInt64Ty(*m_context);

    info = m_source->typeSystem().toType("_builtin_float_");
    info->handle = llvm::Type::getFloatTy(*m_context);

    info = m_source->typeSystem().toType("_builtin_double_");
    info->handle = llvm::Type::getDoubleTy(*m_context);
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

void CodeGen::visit(TypeDecl& node)
{
    if (m_declPass) {
        registerTypeDecl(&node);
        return;
    }
}

void CodeGen::visit(FuncDecl& node)
{
    if (m_declPass) {
        registerFuncDecl(&node);
        return;
    }

    LLVMString name(node.name.toStringRef());
    llvm::Function *f = m_module->getFunction(name);

    int i = 0;
    m_namedValues.clear();
    m_namedTypes.clear();
    for (llvm::Function::arg_iterator it = f->arg_begin(); it != f->arg_end(); ++it, ++i) {
        QSharedPointer<TypeObject> object = node.objects.at(i);
        m_namedValues.insert(object->name.toString(), it);

        TypeInfo* type = m_source->typeSystem().toTypeAndCheck(object->type);
        m_namedTypes.insert(object->name.toString(), type);
    }

    llvm::BasicBlock *block = llvm::BasicBlock::Create(*m_context, "entry", f);
    m_builder->SetInsertPoint(block);
    codegen(node.funcDef.data());
    llvm::verifyFunction(*f);
}

void CodeGen::registerTypeDecl(TypeDecl* node)
{
    node->handle = toCodeGenType(node->name);
}

void CodeGen::registerFuncDecl(FuncDecl* node)
{
    LLVMString name(node->name.toStringRef());
    QList<llvm::Type*> params;
    foreach (QSharedPointer<TypeObject> object, node->objects)
        params.append(toCodeGenType(object->type));

    llvm::Type* returnType = toCodeGenType(node->returnType->type);
    llvm::FunctionType *ft = llvm::FunctionType::get(returnType, params.toVector().toStdVector(), false /*varargs*/);
    llvm::Function *f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, name, m_module.data());

    int i = 0;
    for (llvm::Function::arg_iterator it = f->arg_begin(); it != f->arg_end(); ++it, ++i) {
        QSharedPointer<TypeObject> object = node->objects.at(i);
        LLVMString name(object->name.toStringRef());
        it->setName(name);
    }
}

void CodeGen::codegen(FuncDef* node)
{
    foreach (QSharedPointer<Stmt> stmt, node->stmts)
        codegen(stmt.data());
}

void CodeGen::codegen(Stmt* node)
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
        assert(false); // should not be reached
        return;
    }
}

void CodeGen::codegen(IfStmt* node)
{
    llvm::Value *condition = codegen(node->expr.data());
    assert(condition);

    llvm::Function* f = m_builder->GetInsertBlock()->getParent();
    assert(f);

    llvm::BasicBlock* then = llvm::BasicBlock::Create(*m_context, "then", f);
    llvm::BasicBlock* ifcont = llvm::BasicBlock::Create(*m_context, "ifcont");

    m_builder->CreateCondBr(condition, then, ifcont);

    m_builder->SetInsertPoint(then);

    codegen(node->stmt.data());

    f->getBasicBlockList().push_back(ifcont);
    m_builder->SetInsertPoint(ifcont);
}

void CodeGen::codegen(ReturnStmt* node)
{
    llvm::Function* f = m_builder->GetInsertBlock()->getParent();
    assert(f);

    TypeInfo* funcInfo = m_source->typeSystem().toType(toQString(f->getName()));
    if (!funcInfo) {
        m_source->error(node->keyword, "return statement for function with unknown type", SourceBuffer::Fatal);
        return;
    }

    assert(funcInfo->isNode());
    FuncDecl* funcDecl = static_cast<FuncDecl*>(funcInfo);

    TypeInfo* returnInfo = m_source->typeSystem().toTypeAndCheck(funcDecl->returnType->type);
    if (llvm::Value* value = codegen(node->expr.data(), returnInfo)) {
        m_builder->CreateRet(value);
        return;
    }

    m_source->error(node->keyword, "return statement of void is not allowed", SourceBuffer::Fatal);
}

void CodeGen::codegen(VarDeclStmt* node)
{
    llvm::Function* f = m_builder->GetInsertBlock()->getParent();
    assert(f);

    TypeInfo* info = m_source->typeSystem().toTypeAndCheck(node->type);
    llvm::Value* value = codegen(node->expr.data(), info);
    assert(value);

    QString name = node->name.toString();
    m_namedValues.insert(name, value);
    m_namedTypes.insert(name, info);
}

void CodeGen::comparisonOfSigns(const Token& tok, bool lSigned, bool rSigned)
{
    if (lSigned != rSigned)
        m_source->error(tok, "comparison of signed and unsigned integers not supported", SourceBuffer::Fatal);
}

llvm::Value* CodeGen::codegen(BinaryExpr* node, TypeInfo* info)
{
    llvm::Value* l = 0;
    llvm::Value* r = 0;
    if (!info)
        info = typeInfoForExpr(node);

    if (node->lhs->kind != Node::_LiteralExpr)
        l = codegen(node->lhs.data(), info);

    if (node->rhs->kind != Node::_LiteralExpr)
        r = codegen(node->rhs.data(), info);

    if (!l && !r) {
        m_source->error(static_cast<LiteralExpr*>(node->lhs.data())->literal,
            "we do not support binary expressions involving two literals yet",
            SourceBuffer::Fatal);
        return 0;
    }

    if (!l)
        l = codegen(node->lhs.data(), info);

    if (!r)
        r = codegen(node->rhs.data(), info);

    assert(l && r);

    Token lhs = node->lhs->start;
    if (!l->getType()->isIntegerTy() || !r->getType()->isIntegerTy()) {
        m_source->error(lhs, "non-integer binary expressions not supported yet", SourceBuffer::Fatal);
        return 0;
    }

    // FIXME: This is still incorrect!
    bool lSigned = static_cast<llvm::IntegerType*>(l->getType())->getSignBit();
    bool rSigned = static_cast<llvm::IntegerType*>(r->getType())->getSignBit();

    switch (node->op) {
    case BinaryExpr::OpEquality:
        return m_builder->CreateICmpEQ(l, r, "equaltmp");
    case BinaryExpr::OpLessThanOrEquality:
        comparisonOfSigns(lhs, lSigned, rSigned);
        if (lSigned)
            return m_builder->CreateICmpSLE(l, r, "sletmp");
        else
            return m_builder->CreateICmpULE(l, r, "uletmp");
    case BinaryExpr::OpGreaterThanOrEquality:
        comparisonOfSigns(lhs, lSigned, rSigned);
        if (lSigned)
            return m_builder->CreateICmpSGE(l, r, "sgetmp");
        else
            return m_builder->CreateICmpUGE(l, r, "ugetmp");
    case BinaryExpr::OpLessThan:
        comparisonOfSigns(lhs, lSigned, rSigned);
        if (lSigned)
            return m_builder->CreateICmpSLT(l, r, "slttmp");
        else
            return m_builder->CreateICmpULT(l, r, "ulttmp");
    case BinaryExpr::OpGreaterThan:
        comparisonOfSigns(lhs, lSigned, rSigned);
        if (lSigned)
            return m_builder->CreateICmpSGT(l, r, "sgttmp");
        else
            return m_builder->CreateICmpUGT(l, r, "ugttmp");
    case BinaryExpr::OpAddition:
        return m_builder->CreateAdd(l, r, "addtmp");
    case BinaryExpr::OpSubtraction:
        return m_builder->CreateSub(l, r, "subtmp");
    case BinaryExpr::OpMultiplication:
        return m_builder->CreateMul(l, r, "multmp");
    case BinaryExpr::OpDivision:
        assert(false); // should not be reached
        return 0;
    }
}

llvm::Value* CodeGen::codegen(Expr* node, TypeInfo* info)
{
    switch (node->kind) {
    case Node::_BinaryExpr:
        return codegen(static_cast<BinaryExpr*>(node), info);
    case Node::_FuncCallExpr:
        return codegen(static_cast<FuncCallExpr*>(node), info);
    case Node::_LiteralExpr:
        return codegen(static_cast<LiteralExpr*>(node), info);
    case Node::_VarExpr:
        return codegen(static_cast<VarExpr*>(node), info);
    case Node::_TypeCtorExpr:
        return codegen(static_cast<TypeCtorExpr*>(node), info);
    default:
        assert(false); // should not be reached
        return 0;
    }
}

llvm::Value* CodeGen::codegen(FuncCallExpr* node, TypeInfo* info)
{
    if (!info)
        info = typeInfoForExpr(node);

    LLVMString callee = node->callee.toStringRef();
    llvm::Function *calleeFunction = m_module->getFunction(callee);
    if (calleeFunction->getReturnType() != info->handle) {
        m_source->error(node->callee, "function return type does not match caller", SourceBuffer::Fatal);
        return 0;
    }

    if (!calleeFunction) {
        m_source->error(node->callee, "unknown function reference", SourceBuffer::Fatal);
        return 0;
    }

    if (int(calleeFunction->arg_size()) != node->args.size()) {
        m_source->error(node->callee, "incorrect number of arguments passed", SourceBuffer::Fatal);
        return 0;
    }

    TypeInfo* function = m_source->typeSystem().toTypeAndCheck(node->callee);

    int i = 0;
    QList<llvm::Value*> args;
    QList<TypeRef*> refs = function->typeRefList();
    QList<TypeRef*>::const_iterator end = refs.constEnd();
    for (QList<TypeRef*>::const_iterator it = refs.constBegin(); it != end;  ++it, ++i) {
        QSharedPointer<Expr> arg = node->args.at(i);
        TypeInfo* info = m_source->typeSystem().toType((*it)->typeName());
        llvm::Value* value = codegen(arg.data(), info);
        assert(value);
        args.append(value);
    }

    return m_builder->CreateCall(calleeFunction, args.toVector().toStdVector(), "calltmp");
}

llvm::Value* CodeGen::codegen(LiteralExpr* node, TypeInfo* info)
{
    assert(info);
    assert(info->handle);

    llvm::Type* type = info->handle;
    if (node->literal.type == Digits) {
        if (!type->isIntegerTy()) {
            m_source->error(node->literal, "non-integer literal not supported yet", SourceBuffer::Fatal);
            return 0;
        }

        llvm::IntegerType* integerType = static_cast<llvm::IntegerType*>(type);
        QString digits = node->literal.toString();
        unsigned numbits = integerType->getBitWidth();
        bool success = false;

        if (!info->isSignedInt()) {
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
    assert(false); // should not be reached
    return 0;
}

llvm::Value* CodeGen::codegen(TypeCtorExpr* node, TypeInfo* info)
{
    if (node->type.type == Undefined)
        return codegen(node->args.first().data(), info);
    return 0;
}

llvm::Value* CodeGen::codegen(VarExpr* node, TypeInfo* info)
{
    if (!info)
        info = typeInfoForExpr(node);

    QString name = node->var.toString();
    llvm::Value *value = m_namedValues.contains(name) ? m_namedValues.value(name) : 0;
    if (!value)
        m_source->error(node->var, "unknown variable name", SourceBuffer::Fatal);
    if (!info || info->handle != value->getType())
        m_source->error(node->var, "unknown variable type", SourceBuffer::Fatal);
    return value;
}

llvm::Type* CodeGen::toCodeGenType(const Token& tok) const
{
    TypeInfo* info = m_source->typeSystem().toTypeAndCheck(tok);
    assert(info);
    return info->handle;
}

TypeInfo* CodeGen::typeInfoForExpr(Expr* node) const
{
    switch (node->kind) {
    case Node::_BinaryExpr:
    {
        BinaryExpr* expr = static_cast<BinaryExpr*>(node);

        if (TypeInfo* info = typeInfoForExpr(expr->lhs.data()))
            return info;

        if (TypeInfo* info = typeInfoForExpr(expr->rhs.data()))
            return info;

        m_source->error(expr->lhs->start, "can not determine type for binary expression", SourceBuffer::Fatal);
        return 0;
    }
    case Node::_FuncCallExpr:
    {
        FuncCallExpr* expr = static_cast<FuncCallExpr*>(node);
        TypeInfo* function = m_source->typeSystem().toTypeAndCheck(expr->callee);
        if (TypeRef* returnTypeRef = function->returnTypeRef())
            if (TypeInfo* returnType = m_source->typeSystem().toType(returnTypeRef->typeName()))
                return returnType;

        m_source->error(expr->callee, "can not determine type for function call expression", SourceBuffer::Fatal);
        return 0;
    }
    case Node::_LiteralExpr:
        return 0;
    case Node::_VarExpr:
    {
        VarExpr* expr = static_cast<VarExpr*>(node);
        QString name = expr->var.toString();
        if (m_namedTypes.contains(name))
            return m_namedTypes.value(name);

        m_source->error(expr->var, "can not determine type for variable expression", SourceBuffer::Fatal);
        return 0;
    }
    case Node::_TypeCtorExpr:
    {
        TypeCtorExpr* expr = static_cast<TypeCtorExpr*>(node);
        if (expr->type.type == Undefined)
            return typeInfoForExpr(expr->args.first().data());

        if (TypeInfo* info = m_source->typeSystem().toTypeAndCheck(expr->type))
            return info;

        m_source->error(expr->type, "can not determine type for type ctor expression", SourceBuffer::Fatal);
        return 0;
    }
    default:
        assert(false); // should not be reached
        return 0;
    }
}
