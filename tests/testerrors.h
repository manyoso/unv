#include <QtTest/QtTest>

class TestErrors: public QObject {
    Q_OBJECT
public:
    enum Expectation {
        ExpectSuccess,
        ExpectFailure
    };

    TestErrors() : m_compiler(0) {}

private slots:
    void testSpaceBeforeTab();
    void testTabBeforeSpace();
    void testFuncDeclIsLowerCamelCase();
    void testTypeDeclIsUpperCamelCase();
    void testSingleWhitespace();
    void testInt1Overflow();
    void testInt8Overflow();
    void testInt16Overflow();
    void testInt32Overflow();
    void testInt64Overflow();
    void testUInt8Overflow();
    void testUInt16Overflow();
    void testUInt32Overflow();
    void testUInt64Overflow();
    void testLiteralExprForIfStmt();
    void testTwoLiteralExprForBinaryExpr();
    void testFunctionWithNoReturn();

private:
    void compile(const QString& program, Expectation expect);

private:
    QProcess* m_compiler;
};
