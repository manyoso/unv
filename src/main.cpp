#include <QtCore>

#include "codegen.h"
#include "lexer.h"
#include "output.h"
#include "parser.h"

static bool s_error = false;

void compile(const QString& source, const QString& name)
{
    SourceBuffer buffer(source, name);

    Lexer lexer;
    lexer.lex(&buffer);

    Parser parser;
    parser.parse(&buffer);

    CodeGen codegen(&buffer);
    QString llvmIR = codegen.generateLLVMIR();

    Output output(&buffer);
    output.write(llvmIR);

    s_error = buffer.hasErrors() ? true : s_error;
}

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
            compile(in.readAll(), file.fileName());
            file.close();
        }
    }

    if (Options::instance()->readFromStdin()) {
        QTextStream in(stdin);
        compile(in.readAll(), "stdin");
    }

    return s_error ? EXIT_FAILURE : EXIT_SUCCESS;
}
