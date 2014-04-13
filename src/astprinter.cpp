#include "astprinter.h"

ASTPrinter::ASTPrinter()
    : m_scope(0)
{
}

ASTPrinter::~ASTPrinter()
{
}

QString ASTPrinter::indent() const
{
    return QString(m_scope * 2, ' ');
}

void ASTPrinter::begin(Node& node)
{
    QTextStream out(stdout);
    out << indent() << node.typeToString() << "\n";
    m_scope++;
}

void ASTPrinter::end(Node&)
{
    m_scope--;
}
