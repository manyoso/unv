#include <QtCore>

#include "astprinter.h"
#include "codegen.h"
#include "lexer.h"
#include "parser.h"

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    QStringList args = app.arguments();
    args.removeFirst(); // program name

    // Stop at first error for now while developing the language
    Options::instance()->setErrorLimit(0);

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
