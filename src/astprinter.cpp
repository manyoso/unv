#include "astprinter.h"
#include "sourcebuffer.h"

ASTPrinter::ASTPrinter(SourceBuffer* buffer)
    : m_scope(0)
    , m_stream(new QTextStream(stdout))
    , m_source(buffer)
{
}

ASTPrinter::~ASTPrinter()
{
    delete m_stream;
    m_stream = 0;
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
    *m_stream << indent() << node.typeToString() << "\n";
    m_stream->flush();
    m_scope++;
}

void ASTPrinter::end(Node&)
{
    m_scope--;
}

void ASTPrinter::visit(AliasDecl& node)
{
    *m_stream << indent() << m_source->textForToken(node.type) << " " << m_source->textForToken(node.alias) << "\n";
    m_stream->flush();
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

void ASTPrinter::visit(FuncDeclArg& node)
{
    *m_stream << indent() << m_source->textForToken(node.name) << " " << m_source->textForToken(node.type) << "\n";
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

void ASTPrinter::visit(VarExpr& node)
{
    *m_stream << indent() << m_source->textForToken(node.var) << "\n";
    m_stream->flush();
}
