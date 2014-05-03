#include <QtCore>

#include "testexamples.h"

void TestExamples::testExamples()
{
    QDir examples(QCoreApplication::applicationDirPath() + "/examples");
    QVERIFY(examples.exists());

    QProcess fibonacci;
    fibonacci.setProgram(examples.path() + QDir::separator() + "fibonacci");
    fibonacci.start();
    QVERIFY(fibonacci.waitForFinished());
    QCOMPARE(fibonacci.exitStatus(), QProcess::NormalExit);
    QCOMPARE(fibonacci.exitCode(), 0);
    QCOMPARE(fibonacci.state(), QProcess::NotRunning);

    QProcess numericliterals;
    numericliterals.setProgram(examples.path() + QDir::separator() + "numericliterals");
    numericliterals.start();
    QVERIFY(numericliterals.waitForFinished());
    QCOMPARE(numericliterals.exitStatus(), QProcess::NormalExit);
    QCOMPARE(numericliterals.exitCode(), 0);
    QCOMPARE(numericliterals.state(), QProcess::NotRunning);
}
