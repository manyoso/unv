#include "testparser.h"

#include "astprinter.h"
#include "lexer.h"
#include "parser.h"

void TestParser::testExamples()
{
    QDir examples(QCoreApplication::applicationDirPath() + "/../../examples");
    QVERIFY(examples.exists());
    QFileInfoList unvFiles = examples.entryInfoList(QStringList("*.unv"));
    foreach (QFileInfo f, unvFiles) {
        QFile file(f.filePath());
        QVERIFY(file.exists());
        QVERIFY(file.open(QFile::ReadOnly));
        QTextStream inFile(&file);
        QString fileText = inFile.readAll();
        file.close();

        SourceBuffer buffer(fileText, file.fileName());
        Lexer lexer;
        lexer.lex(&buffer);

        Parser parser;
        parser.parse(&buffer);

        QString astText;
        QTextStream out(&astText);
        ASTPrinter printer(&buffer, &out);
        printer.walk();

        QFile resource(":resources/" + f.baseName() + ".ast");
        if (!resource.exists())
            continue;

        QVERIFY(resource.open(QFile::ReadOnly));
        QTextStream inResource(&resource);
        QString resourceText = inResource.readAll();
        resource.close();

        QCOMPARE(resourceText, astText);
    }
}
