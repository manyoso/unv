#include <QtCore>

#include "lexer.h"

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    QStringList args = app.arguments();
    args.removeFirst(); // program name

    foreach (QString f, args) {
        QFile file(f);
        if (file.open(QFile::ReadOnly)) {
            QTextStream in(&file);
            Lexer lexer(in.readAll());
            lexer.lex();
            lexer.print();
            lexer.printTokens();
        }
    }
}
