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
    QCOMPARE(fibonacci.exitCode(), 203);
    QCOMPARE(fibonacci.state(), QProcess::NotRunning);
}
