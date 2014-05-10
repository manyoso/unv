#include "codegen.h"
#include "filesources.h"
#include "lexer.h"
#include "parser.h"
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
    registerBuiltins();
}

CodeGen::CodeGen(SourceBuffer* buffer, Context context, Module module)
    : m_source(buffer)
    , m_context(context)
    , m_module(module)
    , m_builder(new llvm::Builder(*m_context))
    , m_declPass(true)
{
    registerBuiltins();
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
    return m_includedIR + QString::fromStdString(str);
}

void CodeGen::visit(IncludeDecl& node)
{
    if (!m_declPass)
        return;

    QString include = node.include.toString();
    include.remove(0, 1); // remove leading quote
    include.chop(1); // remove trailing quote

    SourceBuffer* buffer = FileSources::instance()->sourceBuffer(include);
    if (!buffer) {
        m_source->error(node.include, "Could not find or open include file", SourceBuffer::Fatal);
        return;
    }

    Lexer lexer;
    lexer.lex(buffer);

    Parser parser;
    parser.parse(buffer);

    CodeGen codegen(buffer, m_context, m_module);
    m_includedIR += codegen.generateLLVMIR();

    m_source->typeSystem().importTypes(buffer->typeSystem());
    m_source->addErrors(buffer->numberOfErrors());
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
    m_source->typeSystem().clearNamedTypes();
    for (llvm::Function::arg_iterator it = f->arg_begin(); it != f->arg_end(); ++it, ++i) {
        QSharedPointer<TypeObject> object = node.objects.at(i);
        m_namedValues.insert(object->name.toString(), it);

        TypeInfo* type = m_source->typeSystem().toTypeAndCheck(object->type);
        m_source->typeSystem().insertNamedType(object->name.toString(), type);
    }

    llvm::BasicBlock *block = llvm::BasicBlock::Create(*m_context, "entry", f);
    m_builder->SetInsertPoint(block);
    codegen(node.funcDef.data());

    if (node.funcDef->stmts.last()->kind != Node::_ReturnStmt)
        m_source->error(node.name, "function must end with return statement", SourceBuffer::Fatal);

    llvm::verifyFunction(*f);
}

void CodeGen::registerBuiltins()
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
    if (node->expr->kind == Node::_LiteralExpr) {
        m_source->error(node->expr->start,
            "literal expression can not be used as the only expression of an if statement",
            SourceBuffer::Fatal);
        return;
    }

    TypeInfo* info = m_source->typeSystem().typeInfoForExpr(node->expr.data());
    assert(info);
    llvm::Value *condition = codegen(node->expr.data(), info);
    assert(condition);

    if (condition->getType() != llvm::Type::getInt1Ty(*m_context)) {
        m_source->error(node->expr->start,
            "expression in if statement does not evaluate to true or false",
            SourceBuffer::Fatal);
    }

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
    m_source->typeSystem().insertNamedType(name, info);
}

llvm::Value* CodeGen::codegen(BinaryExpr* node, TypeInfo* info)
{
    llvm::Value* l = 0;
    llvm::Value* r = 0;
    if (!info)
        info = m_source->typeSystem().typeInfoForExpr(node);

    if (node->lhs->kind != Node::_LiteralExpr)
        l = codegen(node->lhs.data(), info);

    if (node->rhs->kind != Node::_LiteralExpr)
        r = codegen(node->rhs.data(), info);

    if (!l && !r) {
        m_source->error(static_cast<LiteralExpr*>(node->lhs.data())->literal,
            "we do not support binary expressions involving two literals",
            SourceBuffer::Fatal);
        return 0;
    }

    if (!l)
        l = codegen(node->lhs.data(), info);

    if (!r)
        r = codegen(node->rhs.data(), info);

    assert(l && r);

    m_source->typeSystem().checkCompatibleTypes(node->lhs.data(), node->rhs.data());

    TypeInfo* infoForLHS = m_source->typeSystem().typeInfoForExpr(node->lhs.data());
    TypeInfo* infoForRHS = m_source->typeSystem().typeInfoForExpr(node->rhs.data());
    TypeInfo* infoForExpressions = infoForLHS ? infoForLHS : infoForRHS;

    assert(infoForExpressions);
    assert(infoForExpressions->handle);

    bool isInteger = infoForExpressions->handle->isIntegerTy();
    bool isSignedInteger = infoForExpressions->isSignedInt();
    bool isFloat = infoForExpressions->handle->isFloatTy();
    bool isDouble = infoForExpressions->handle->isDoubleTy();

    // FIXME: Still need to take ordered vs unordered comparisons into
    // account for floating point types
    switch (node->op) {
    case BinaryExpr::OpEquality:
        if (isInteger)
            return m_builder->CreateICmpEQ(l, r, "equaltmp");
        else if (isFloat || isDouble)
            return m_builder->CreateFCmpOEQ(l, r, "oequaltmp");
    case BinaryExpr::OpNotEquality:
        if (isInteger)
            return m_builder->CreateICmpNE(l, r, "notequaltmp");
        else if (isFloat || isDouble)
            return m_builder->CreateFCmpONE(l, r, "onotequaltmp");
    case BinaryExpr::OpLessThanOrEquality:
        if (isInteger) {
            if (isSignedInteger)
                return m_builder->CreateICmpSLE(l, r, "sletmp");
            else
                return m_builder->CreateICmpULE(l, r, "uletmp");
        } else if (isFloat || isDouble)
            return m_builder->CreateFCmpOLE(l, r, "oletmp");
    case BinaryExpr::OpGreaterThanOrEquality:
        if (isInteger) {
            if (isSignedInteger)
                return m_builder->CreateICmpSGE(l, r, "sgetmp");
            else
                return m_builder->CreateICmpUGE(l, r, "ugetmp");
        } else if (isFloat || isDouble)
            return m_builder->CreateFCmpOGE(l, r, "ogetmp");
    case BinaryExpr::OpLessThan:
        if (isInteger) {
            if (isSignedInteger)
                return m_builder->CreateICmpSLT(l, r, "slttmp");
            else
                return m_builder->CreateICmpULT(l, r, "ulttmp");
        } else if (isFloat || isDouble)
            return m_builder->CreateFCmpOLT(l, r, "olttmp");
    case BinaryExpr::OpGreaterThan:
        if (isInteger) {
            if (isSignedInteger)
                return m_builder->CreateICmpSGT(l, r, "sgttmp");
            else
                return m_builder->CreateICmpUGT(l, r, "ugttmp");
        } else if (isFloat || isDouble)
            return m_builder->CreateFCmpOGT(l, r, "ogttmp");
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
        info = m_source->typeSystem().typeInfoForExpr(node);

    LLVMString callee = node->callee.toStringRef();
    llvm::Function *calleeFunction = m_module->getFunction(callee);
    if (!calleeFunction) {
        m_source->error(node->callee, "unknown function reference", SourceBuffer::Fatal);
        return 0;
    }

    if (calleeFunction->getReturnType() != info->handle) {
        m_source->error(node->callee, "function return type does not match caller", SourceBuffer::Fatal);
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

static int integerTypeToBase(TokenType type)
{
    switch (type) {
    case BinLiteral: return 2;
    case OctLiteral: return 8;
    case DecLiteral: return 10;
    case HexLiteral: return 16;
    default:
        assert(false); // should not be reached
        return 0;
    }
}

static QString integerLiteralToString(Token token)
{
    QString literal = token.toString();
    if (token.type == BinLiteral || token.type == HexLiteral) {
        if (!literal.startsWith('-'))
            return literal.remove(0, 2);
        else
            return literal.remove(1, 2);
    }
    return literal;
}

llvm::Value* CodeGen::codegen(LiteralExpr* node, TypeInfo* info)
{
    assert(info);

    if (node->literal.type == True) {
        return llvm::ConstantInt::getTrue(*m_context);
    } else if (node->literal.type == False) {
        return llvm::ConstantInt::getFalse(*m_context);
    } else if (node->literal.type == StringLiteral) {
        QString literal = node->literal.toString();
        literal.remove(0, 1); // remove leading quote
        literal.chop(1); // remove trailing quote
        llvm::Constant* constant = llvm::ConstantDataArray::getString(*m_context, LLVMString(literal));
        info->handle = constant->getType();
        return constant;
    } else if (node->literal.type == FloatLiteral) {
        assert(info->handle);
        llvm::Type* type = info->handle;
        assert(type->isFloatTy() || type->isDoubleTy());

        QString literal = node->literal.toString();
        if (type->isFloatTy()) {
            bool success = false;
            float f = literal.toFloat(&success);

            bool outOfRange = f > std::numeric_limits<float>::max();

            if (!success || outOfRange || !llvm::ConstantFP::isValueValidForType(type, llvm::APFloat(f))) {
                m_source->error(node->literal, "float literal out of range", SourceBuffer::Fatal);
                return 0;
            }

            return llvm::ConstantFP::get(*m_context, llvm::APFloat(f));
        } else {
            bool success = false;
            double d = literal.toDouble(&success);

            bool outOfRange = d > std::numeric_limits<double>::max();

            if (!success || outOfRange || !llvm::ConstantFP::isValueValidForType(type, llvm::APFloat(d))) {
                m_source->error(node->literal, "double literal out of range", SourceBuffer::Fatal);
                return 0;
            }

            return llvm::ConstantFP::get(*m_context, llvm::APFloat(d));
        }
    } else if (node->literal.type == BinLiteral
        || node->literal.type == DecLiteral
        || node->literal.type == HexLiteral
        || node->literal.type == OctLiteral ) {

        assert(info->handle);
        llvm::Type* type = info->handle;
        if (!type->isIntegerTy())
            m_source->error(node->literal, "expression for integer literal has incompatible type", SourceBuffer::Fatal);

        llvm::IntegerType* integerType = static_cast<llvm::IntegerType*>(type);
        QString digits = integerLiteralToString(node->literal);
        unsigned numbits = integerType->getBitWidth();
        bool success = false;

        if (!info->isSignedInt()) {
            quint64 n = digits.toULongLong(&success, integerTypeToBase(node->literal.type));

            if (!success || !llvm::ConstantInt::isValueValidForType(type, n)) {
                m_source->error(node->literal, "unsigned integer literal out of range", SourceBuffer::Fatal);
                return 0;
            }

            return llvm::ConstantInt::get(*m_context, llvm::APInt(numbits, n, false));
        } else {
            qint64 n = digits.toLongLong(&success, integerTypeToBase(node->literal.type));

            if (!success || !llvm::ConstantInt::isValueValidForType(type, n)) {
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
        info = m_source->typeSystem().typeInfoForExpr(node);

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
    assert(info->handle);
    return info->handle;
}
