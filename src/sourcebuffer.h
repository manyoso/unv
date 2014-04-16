#ifndef sourcebuffer_h
#define sourcebuffer_h

#include <QtCore>

#include "ast.h"
#include "options.h"
#include "symbols.h"
#include "token.h"

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
        m_symbols = QSharedPointer<Symbols>(new Symbols(this));
        m_hasErrors = false;
    }

    QString name() const { return m_name; }

    QString module() const
    {
        QFileInfo info(m_name);
        return info.baseName();
    }

    QChar at(int index) const
    { return m_source.at(index); }

    QString text(int pos, int n) const
    { return m_source.mid(pos, n); }

    QString lineForToken(const Token& tok) const
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
        return m_source.mid(start, end - start + 1);
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

    QString textForToken(const Token& tok) const
    {
        int start = indexForPosition(tok.start);
        int end = indexForPosition(tok.end);
        return m_source.mid(start, end - start + 1);
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
            stream << textForToken(tok);
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
        static int s_numberOfErrors = 0;
        Q_ASSERT(tok.type != Undefined);
        if (type == Error)
            s_numberOfErrors++;
        QString err = type == Error ? "error" : "fatal error";
        QString location = name()
            + ":" + QString::number(tok.start.line)
            + ":" + QString::number(tok.start.column)
#ifdef Q_OS_UNIX
            + "\033[91m " + err + "\033[39m: " + str;
#else
            + " " + err + ": " + str;
#endif
        QString context = lineForToken(tok);
        context.replace('\n', QChar());
        QString caret(tok.start.column - 1, ' ');
        context.replace('\t', ' ');
        caret.replace('\t', ' ');
#ifdef Q_OS_UNIX
        caret += "\033[92m" + QString(tok.end.column - tok.start.column + 1, '^') + "\033[39m";
#else
        caret += QString(tok.end.column - tok.start.column + 1, '^');
#endif

        m_hasErrors = true;

        QTextStream out(stderr);
        out << location << '\n' << context << '\n' << caret << '\n';
        if (type == Fatal || s_numberOfErrors > Options::instance()->errorLimit()) {
            out.flush();
            exit(EXIT_FAILURE);
        }
    }

    TranslationUnit& translationUnit() const { return *m_translationUnit; }

    Symbols& symbols() const { return *m_symbols; }

    bool hasErrors() const { return m_hasErrors; }

private:
    QString m_source;
    QString m_name;
    QList<Token> m_tokens;
    QList<int> m_lineInfo;
    QSharedPointer<TranslationUnit> m_translationUnit;
    QSharedPointer<Symbols> m_symbols;
    bool m_hasErrors;
};

#endif // sourcebuffer_h
