#include <QtCore>

#include "astprinter.h"
#include "codegen.h"
#include "lexer.h"
#include "parser.h"

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("unv");
    QCoreApplication::setApplicationVersion("0.1");

    Options::instance()->parseCommandLine();

    QStringList args = Options::instance()->files();

    foreach (QString f, args) {
        QFile file(f);
        if (file.open(QFile::ReadOnly)) {
            QTextStream in(&file);
            SourceBuffer buffer(in.readAll(), file.fileName());

            Lexer lexer;
            lexer.lex(&buffer);

            Parser parser;
            parser.parse(&buffer);

            ASTPrinter printer(&buffer);
            printer.walk();

            CodeGen codegen(&buffer);
            codegen.walk();
        }
    }
}
