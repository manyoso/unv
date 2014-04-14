#include "lexer.h"
#include "testlexer.h"

void TestLexer::testExamples()
{
    QDir examples(QCoreApplication::applicationDirPath() + "/../../examples");
    QVERIFY(examples.exists());
    QFileInfoList unvFiles = examples.entryInfoList(QStringList("*.unv"));
    foreach (QFileInfo f, unvFiles) {
        QFile file(f.filePath());
        QVERIFY(file.exists());
        QVERIFY(file.open(QFile::ReadOnly));
        QTextStream in(&file);
        QString fileText = in.readAll();
        file.close();

        SourceBuffer buffer(fileText, file.fileName());
        Lexer lexer;
        lexer.lex(&buffer);

        QString lexText;
        QTextStream stream(&lexText);
        buffer.print(stream);

        QCOMPARE(fileText, lexText);
    }
}
