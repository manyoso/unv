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
            SourceBuffer buffer(in.readAll(), file.fileName());
            Lexer lexer;
            lexer.lex(&buffer);
            buffer.print();
            buffer.printTokens();
        }
    }
}
