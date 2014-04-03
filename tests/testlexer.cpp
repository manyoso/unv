#include <QtTest/QtTest>

#include "lexer.h"
#include "testlexer.h"

void TestLexer::test()
{
    QString test = "hello";
    Lexer lexer(test);
    lexer.lex();
    QString text;
    QTextStream stream(&text);
    lexer.print(stream);
    QCOMPARE(test, text);
}
