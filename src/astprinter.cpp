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
    *m_stream << indent() << m_source->textForToken(node.callee) << "\n";
    m_stream->flush();
}

void ASTPrinter::visit(FuncDecl& node)
{
    *m_stream << indent() << m_source->textForToken(node.name) << " " << m_source->textForToken(node.returnType) << "\n";
    m_stream->flush();
}

void ASTPrinter::visit(LiteralExpr& node)
{
    *m_stream << indent() << m_source->textForToken(node.literal) << "\n";
    m_stream->flush();
}

void ASTPrinter::visit(TypeDecl& node)
{
    *m_stream << indent() << m_source->textForToken(node.name) << "\n";
    m_stream->flush();
}

void ASTPrinter::visit(TypeObject& node)
{
    if (node.name.type != Undefined)
        *m_stream << indent() << m_source->textForToken(node.name) << " " << m_source->textForToken(node.type) << "\n";
    else
        *m_stream << indent() << m_source->textForToken(node.type) << "\n";
    m_stream->flush();
}

void ASTPrinter::visit(VarExpr& node)
{
    *m_stream << indent() << m_source->textForToken(node.var) << "\n";
    m_stream->flush();
}

void ASTPrinter::visit(VarDeclStmt& node)
{
    *m_stream << indent() << m_source->textForToken(node.type) << " " << m_source->textForToken(node.name) << "\n";
    m_stream->flush();
}
