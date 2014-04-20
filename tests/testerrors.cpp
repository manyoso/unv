#include "testerrors.h"

void TestErrors::compile(const QString& program, Expectation expect)
{
    m_compiler = new QProcess;
    m_compiler->setProgram(QCoreApplication::applicationDirPath() + "/unv");
    m_compiler->setArguments(QStringList() << "-e" << "llvm" << "-stdin");
    m_compiler->start();
    QVERIFY(m_compiler->waitForStarted());
    m_compiler->write(program.toLatin1());
    m_compiler->waitForBytesWritten();
    m_compiler->closeWriteChannel();
    QVERIFY(m_compiler->waitForFinished());
    QCOMPARE(m_compiler->exitStatus(), QProcess::NormalExit);
    QCOMPARE(m_compiler->exitCode(), expect == ExpectFailure ? EXIT_FAILURE : EXIT_SUCCESS);
    QCOMPARE(m_compiler->state(), QProcess::NotRunning);
    delete m_compiler;
    m_compiler = 0;
}

void TestErrors::testSpaceBeforeTab()
{
    compile("    \n\t", ExpectFailure);
}

void TestErrors::testTabBeforeSpace()
{
    compile("\t\n    ", ExpectFailure);
}

void TestErrors::testFuncDeclIsLowerCamelCase()
{
    compile("function Main : () -> Int", ExpectFailure);
}

void TestErrors::testTypeDeclIsUpperCamelCase()
{
    compile("type int : (_builtin_int32_)", ExpectFailure);
}

void TestErrors::testSingleWhitespace()
{
    compile("function  main : () -> Int", ExpectFailure);
}

void TestErrors::testInt1Overflow()
{
    QString testInt1 = "function main : () -> _builtin_bit_\n\treturn ";
    compile(testInt1 + "1", ExpectSuccess);
    compile(testInt1 + "0", ExpectSuccess);
    compile(testInt1 + "2", ExpectFailure);
    compile(testInt1 + "-1", ExpectFailure);
}

void TestErrors::testInt8Overflow()
{
    QString testInt8 = "function main : () -> _builtin_int8_\n\treturn ";
    QString maxInt8 = QString::number(127);
    QString minInt8 = QString::number(-128);
    compile(testInt8 + maxInt8, ExpectSuccess);
    compile(testInt8 + minInt8, ExpectSuccess);

    {
        unsigned lastDigit = QString(maxInt8.at(maxInt8.count() - 1)).toInt();
        lastDigit++;
        maxInt8.replace(maxInt8.count() - 1, 1, QString::number(lastDigit));
        compile(testInt8 + maxInt8, ExpectFailure);
    }

    {
        unsigned lastDigit = QString(minInt8.at(minInt8.count() - 1)).toInt();
        lastDigit++;
        minInt8.replace(minInt8.count() - 1, 1, QString::number(lastDigit));
        compile(testInt8 + minInt8, ExpectFailure);
    }
}

void TestErrors::testInt16Overflow()
{
    QString testInt16 = "function main : () -> _builtin_int16_\n\treturn ";
    QString maxInt16 = QString::number(std::numeric_limits<int16_t>::max());
    QString minInt16 = QString::number(std::numeric_limits<int16_t>::min());
    compile(testInt16 + maxInt16, ExpectSuccess);
    compile(testInt16 + minInt16, ExpectSuccess);

    {
        unsigned lastDigit = QString(maxInt16.at(maxInt16.count() - 1)).toInt();
        lastDigit++;
        maxInt16.replace(maxInt16.count() - 1, 1, QString::number(lastDigit));
        compile(testInt16 + maxInt16, ExpectFailure);
    }

    {
        unsigned lastDigit = QString(minInt16.at(minInt16.count() - 1)).toInt();
        lastDigit++;
        minInt16.replace(minInt16.count() - 1, 1, QString::number(lastDigit));
        compile(testInt16 + minInt16, ExpectFailure);
    }
}

void TestErrors::testInt32Overflow()
{
    QString testInt32 = "function main : () -> _builtin_int32_\n\treturn ";
    QString maxInt32 = QString::number(std::numeric_limits<int32_t>::max());
    QString minInt32 = QString::number(std::numeric_limits<int32_t>::min());
    compile(testInt32 + maxInt32, ExpectSuccess);
    compile(testInt32 + minInt32, ExpectSuccess);

    {
        unsigned lastDigit = QString(maxInt32.at(maxInt32.count() - 1)).toInt();
        lastDigit++;
        maxInt32.replace(maxInt32.count() - 1, 1, QString::number(lastDigit));
        compile(testInt32 + maxInt32, ExpectFailure);
    }

    {
        unsigned lastDigit = QString(minInt32.at(minInt32.count() - 1)).toInt();
        lastDigit++;
        minInt32.replace(minInt32.count() - 1, 1, QString::number(lastDigit));
        compile(testInt32 + minInt32, ExpectFailure);
    }
}

void TestErrors::testInt64Overflow()
{
    QString testInt64 = "function main : () -> _builtin_int64_\n\treturn ";
    QString maxInt64 = QString::number(std::numeric_limits<int64_t>::max());
    QString minInt64 = QString::number(std::numeric_limits<int64_t>::min());
    compile(testInt64 + maxInt64, ExpectSuccess);
    compile(testInt64 + minInt64, ExpectSuccess);

    {
        unsigned lastDigit = QString(maxInt64.at(maxInt64.count() - 1)).toInt();
        lastDigit++;
        maxInt64.replace(maxInt64.count() - 1, 1, QString::number(lastDigit));
        compile(testInt64 + maxInt64, ExpectFailure);
    }

    {
        unsigned lastDigit = QString(minInt64.at(minInt64.count() - 1)).toInt();
        lastDigit++;
        minInt64.replace(minInt64.count() - 1, 1, QString::number(lastDigit));
        compile(testInt64 + minInt64, ExpectFailure);
    }
}
