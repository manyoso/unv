#include <QtCore>
#include <QtTest/QtTest>

#include "testlexer.h"
#include "testexamples.h"
#include "testparser.h"

#define _NAME_ "unvtests"

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(_NAME_);

    int rc = 0;
    TestLexer test1;
    rc = QTest::qExec(&test1, argc, argv) == 0 ? rc : -1;

    TestParser test2;
    rc = QTest::qExec(&test2, argc, argv) == 0 ? rc : -1;

    TestExamples test3;
    rc = QTest::qExec(&test3, argc, argv) == 0 ? rc : -1;

    return rc;
}
