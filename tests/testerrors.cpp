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

void TestErrors::testUInt8Overflow()
{
    QString testUInt8 = "function main : () -> _builtin_uint8_\n\treturn ";
    QString maxUInt8 = QString::number(255);
    QString minUInt8 = QString::number(0);
    compile(testUInt8 + maxUInt8, ExpectSuccess);
    compile(testUInt8 + minUInt8, ExpectSuccess);

    {
        unsigned lastDigit = QString(maxUInt8.at(maxUInt8.count() - 1)).toUInt();
        lastDigit++;
        maxUInt8.replace(maxUInt8.count() - 1, 1, QString::number(lastDigit));
        compile(testUInt8 + maxUInt8, ExpectFailure);
    }

    {
        minUInt8 = QString("-1");
        compile(testUInt8 + minUInt8, ExpectFailure);
    }
}

void TestErrors::testUInt16Overflow()
{
    QString testUInt16 = "function main : () -> _builtin_uint16_\n\treturn ";
    QString maxUInt16 = QString::number(std::numeric_limits<uint16_t>::max());
    QString minUInt16 = QString::number(std::numeric_limits<uint16_t>::min());
    compile(testUInt16 + maxUInt16, ExpectSuccess);
    compile(testUInt16 + minUInt16, ExpectSuccess);

    {
        unsigned lastDigit = QString(maxUInt16.at(maxUInt16.count() - 1)).toUInt();
        lastDigit++;
        maxUInt16.replace(maxUInt16.count() - 1, 1, QString::number(lastDigit));
        compile(testUInt16 + maxUInt16, ExpectFailure);
    }

    {
        minUInt16 = QString("-1");
        compile(testUInt16 + minUInt16, ExpectFailure);
    }
}

void TestErrors::testUInt32Overflow()
{
    QString testUInt32 = "function main : () -> _builtin_uint32_\n\treturn ";
    QString maxUInt32 = QString::number(std::numeric_limits<uint32_t>::max());
    QString minUInt32 = QString::number(std::numeric_limits<uint32_t>::min());
    compile(testUInt32 + maxUInt32, ExpectSuccess);
    compile(testUInt32 + minUInt32, ExpectSuccess);

    {
        unsigned lastDigit = QString(maxUInt32.at(maxUInt32.count() - 1)).toUInt();
        lastDigit++;
        maxUInt32.replace(maxUInt32.count() - 1, 1, QString::number(lastDigit));
        compile(testUInt32 + maxUInt32, ExpectFailure);
    }

    {
        minUInt32 = QString("-1");
        compile(testUInt32 + minUInt32, ExpectFailure);
    }
}

void TestErrors::testUInt64Overflow()
{
    QString testUInt64 = "function main : () -> _builtin_uint64_\n\treturn ";
    QString maxUInt64 = QString::number(std::numeric_limits<uint64_t>::max());
    QString minUInt64 = QString::number(std::numeric_limits<uint64_t>::min());
    compile(testUInt64 + maxUInt64, ExpectSuccess);
    compile(testUInt64 + minUInt64, ExpectSuccess);

    {
        unsigned lastDigit = QString(maxUInt64.at(maxUInt64.count() - 1)).toInt();
        lastDigit++;
        maxUInt64.replace(maxUInt64.count() - 1, 1, QString::number(lastDigit));
        compile(testUInt64 + maxUInt64, ExpectFailure);
    }

    {
        minUInt64 = QString("-1");
        compile(testUInt64 + minUInt64, ExpectFailure);
    }
}

void TestErrors::testLiteralExprForIfStmt()
{
    compile("type Int : (_builtin_int32_)\nfunction main : () -> Int\n\tif (0) return 1\n\treturn 0", ExpectFailure);
}
