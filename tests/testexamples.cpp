#include <QtTest/QtTest>
#include <QtCore>

#include "testexamples.h"

void TestExamples::testExamples()
{
    QDir examples(QCoreApplication::applicationDirPath() + "/../../build/bin/examples");
    QVERIFY(examples.exists());

    QProcess fibonacci;
    fibonacci.setProgram(examples.path() + QDir::separator() + "fibonacci");
    fibonacci.start();
    QVERIFY(fibonacci.waitForFinished());
    QCOMPARE(fibonacci.exitStatus(), QProcess::NormalExit);
    QCOMPARE(fibonacci.exitCode(), 0);
    QCOMPARE(fibonacci.state(), QProcess::NotRunning);
}
