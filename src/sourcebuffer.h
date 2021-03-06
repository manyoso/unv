#ifndef sourcebuffer_h
#define sourcebuffer_h

#include <QtCore>

#include "assert.h"
#include "ast.h"
#include "options.h"
#include "token.h"
#include "typesystem.h"

class SourceBuffer {
public:
    enum ErrorType {
        Error,
        Fatal
    };

    SourceBuffer(const QString& source, const QString& name = "")
    {
        m_source = source;
        m_name = name;
        m_translationUnit = QSharedPointer<TranslationUnit>(new TranslationUnit);
        m_typeSystem = QSharedPointer<TypeSystem>(new TypeSystem(this));
        m_numberOfErrors = 0;
        m_isCompiled = false;
    }

    QString name() const { return m_name; }

    QString module() const
    {
        QFileInfo info(m_name);
        return info.baseName();
    }

    QChar at(int index) const
    { return m_source.at(index); }

    QStringRef text(int pos, int n) const
    { return m_source.midRef(pos, n); }

    QStringRef lineForToken(const Token& tok) const
    {
        int start = 0;
        int end = 0;
        if (tok.start.line - 2 >= 0)
            start = m_lineInfo.at(tok.start.line - 2);

        if (tok.start.line - 1 >= m_lineInfo.count()) {
            int newline = m_source.indexOf('\n', start);
            if (newline != -1)
                end = newline - 1;
            else
                end = m_source.count() - 1;
        } else {
            end = m_lineInfo.at(tok.start.line - 1);
        }
        return m_source.midRef(start, end - start + 1);
    }

    int count() const
    { return m_source.count(); }

    int indexForPosition(const TokenPosition& pos) const
    {
        int line = pos.line - 1;
        int column = pos.column - 1;
        return !line ? column : m_lineInfo.at(line - 1) + column;
    }

    void appendNewline(int index)
    { m_lineInfo.append(index); }

    int newlineCount() const
    { return m_lineInfo.count(); }

    void appendToken(const Token& token)
    { m_tokens.append(token); }

    QStringRef textForTokenPosition(const TokenPosition& s, const TokenPosition& e) const
    {
        int start = indexForPosition(s);
        int end = indexForPosition(e);
        return m_source.midRef(start, end - start + 1);
    }

    Token tokenAt(int index) const
    { return m_tokens.at(index); }

    int tokenCount() const
    { return m_tokens.count(); }

    void print() const
    {
        QTextStream out(stdout);
        print(out);
        out << "\n";
    }

    void print(QTextStream& stream) const
    {
        foreach (Token tok, m_tokens)
            stream << tok.toStringRef().toString();
    }

    void printTokens() const
    {
        QTextStream out(stdout);
        printTokens(out);
    }

    void printTokens(QTextStream& stream) const
    {
        foreach (Token tok, m_tokens) {

            QString range = QString("start(line:%1|column:%2), end(line:%3|column%4)")
                .arg(QString::number(tok.start.line))
                .arg(QString::number(tok.start.column))
                .arg(QString::number(tok.end.line))
                .arg(QString::number(tok.end.column));
            stream << typeToString(tok.type) << ": " << range << "\n";
        }
    }

    void error(const Token& tok, const QString& str, ErrorType type = Error)
    {
        assert(tok.start.line != -1 && tok.start.column != -1 && tok.end.line != -1 && tok.end.column != -1);

        if (type == Error)
            m_numberOfErrors++;
        QString err = type == Error ? "error" : "fatal error";
        QString location = name()
            + ":" + QString::number(tok.start.line)
            + ":" + QString::number(tok.start.column)
#ifdef Q_OS_UNIX
            + "\033[91m " + err + "\033[39m: " + str;
#else
            + " " + err + ": " + str;
#endif
        QString context = lineForToken(tok).toString();
        context.replace('\n', QChar());
        QString caret(tok.start.column - 1, ' ');
        context.replace('\t', ' ');
        caret.replace('\t', ' ');
#ifdef Q_OS_UNIX
        caret += "\033[92m" + QString(tok.end.column - tok.start.column + 1, '^') + "\033[39m";
#else
        caret += QString(tok.end.column - tok.start.column + 1, '^');
#endif

        QTextStream out(stderr);
        out << location << '\n' << context << '\n' << caret << '\n';
        if (type == Fatal || m_numberOfErrors > Options::instance()->errorLimit()) {
            out.flush();
            exit(EXIT_FAILURE);
        }
    }

    TranslationUnit& translationUnit() const { return *m_translationUnit; }

    TypeSystem& typeSystem() const { return *m_typeSystem; }

    bool hasErrors() const { return m_numberOfErrors > 0; }
    int numberOfErrors() const { return m_numberOfErrors; }
    void addErrors(int errors) { m_numberOfErrors += errors; }
    bool isCompiled() const { return m_isCompiled; }
    void setCompiled(bool compiled) { m_isCompiled = compiled; }

private:
    QString m_source;
    QString m_name;
    QList<Token> m_tokens;
    QList<int> m_lineInfo;
    QSharedPointer<TranslationUnit> m_translationUnit;
    QSharedPointer<TypeSystem> m_typeSystem;
    int m_numberOfErrors;
    bool m_isCompiled;
};

#endif // sourcebuffer_h
