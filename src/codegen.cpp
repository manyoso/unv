#include "codegen.h"
#include "sourcebuffer.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#pragma clang diagnostic pop

CodeGen::CodeGen(SourceBuffer* buffer)
    : m_source(buffer)
    , m_context(new llvm::LLVMContext)
    , m_module(new llvm::Module("foo", (*m_context)))
    , m_builder(new llvm::Builder(*m_context))
{
}

CodeGen::~CodeGen()
{
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

void CodeGen::visit(FuncDecl&)
{
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
