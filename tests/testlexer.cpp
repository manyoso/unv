#include <QtTest/QtTest>

#include "lexer.h"
#include "testlexer.h"

void TestLexer::testExamples()
{
    QDir examples(QCoreApplication::applicationDirPath() + "/../../examples");
    QVERIFY(examples.exists());
    QStringList unvFiles = examples.entryList(QStringList("*.unv"));
    foreach (QString f, unvFiles) {
        QFile file(f);
        if (file.open(QFile::ReadOnly)) {
            QTextStream in(&file);
            QString fileText = in.readAll();
            SourceBuffer buffer(fileText, file.fileName());
            Lexer lexer;
            lexer.lex(&buffer);
            QString lexText;
            QTextStream stream(&lexText);
            buffer.print(stream);
            QCOMPARE(fileText, lexText);
        }
    }
}
