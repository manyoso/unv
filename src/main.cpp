#include <QtCore>

#include "codegen.h"
#include "lexer.h"
#include "output.h"
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

            CodeGen codegen(&buffer);
            QString llvmIR = codegen.generateLLVMIR();

            Output output(&buffer);
            output.write(llvmIR);
        }
    }
}
