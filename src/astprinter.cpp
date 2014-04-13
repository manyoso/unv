#include "astprinter.h"

void ASTPrinter::visit(Node& node)
{
    QTextStream out(stdout);
    out << "node:" << node.typeToString() << "\n";
}
