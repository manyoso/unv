#include "astprinter.h"
#include "sourcebuffer.h"

ASTPrinter::ASTPrinter(SourceBuffer* buffer, QTextStream* stream)
    : m_scope(0)
    , m_stream(stream)
    , m_source(buffer)
{
}

ASTPrinter::~ASTPrinter()
{
}

void ASTPrinter::walk()
{
    Visitor::walk(m_source->translationUnit());
}

QString ASTPrinter::indent() const
{
    return QString(m_scope * 2, ' ');
}

void ASTPrinter::begin(Node& node)
{
    *m_stream << indent() << node.kindToString() << "\n";
    m_stream->flush();
    m_scope++;
}

void ASTPrinter::end(Node&)
{
    m_scope--;
}

void ASTPrinter::visit(BinaryExpr& node)
{
    *m_stream << indent() << node.opToString() << "\n";
    m_stream->flush();
}

void ASTPrinter::visit(FuncCallExpr& node)
{
    *m_stream << indent() << node.callee.toString() << "\n";
    m_stream->flush();
}

void ASTPrinter::visit(FuncDecl& node)
{
    *m_stream << indent() << node.name.toString() << "\n";
    m_stream->flush();
}

void ASTPrinter::visit(LiteralExpr& node)
{
    *m_stream << indent() << node.literal.toString() << "\n";
    m_stream->flush();
}

void ASTPrinter::visit(TypeCtorExpr& node)
{
    if (node.type.type != Undefined)
        *m_stream << indent() << node.type.toString() << "\n";
    m_stream->flush();
}

void ASTPrinter::visit(TypeDecl& node)
{
    *m_stream << indent() << node.name.toString() << "\n";
    m_stream->flush();
}

void ASTPrinter::visit(TypeObject& node)
{
    if (node.name.type != Undefined)
        *m_stream << indent() << node.name.toString() << " " << node.type.toString() << "\n";
    else
        *m_stream << indent() << node.type.toString() << "\n";
    m_stream->flush();
}

void ASTPrinter::visit(VarExpr& node)
{
    *m_stream << indent() << node.var.toString() << "\n";
    m_stream->flush();
}

void ASTPrinter::visit(VarDeclStmt& node)
{
    *m_stream << indent() << node.type.toString() << " " << node.name.toString() << "\n";
    m_stream->flush();
}
