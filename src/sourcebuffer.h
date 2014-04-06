#ifndef sourcebuffer_h
#define sourcebuffer_h

#include <QtCore>

#include "token.h"

class SourceBuffer {
public:
    enum ErrorType {
        Error,
        Fatal
    };

    SourceBuffer(const QString& source, const QString& name = "")
    { m_source = source; m_name = name; }

    QString name() const { return m_name; }

    QChar at(int index) const
    { return m_source.at(index); }

    QString text(int pos, int n) const
    { return m_source.mid(pos, n); }

    QString lineForToken(const Token& tok) const
    {
        int start = m_lineInfo.at(tok.start.line - 2);
        int end = tok.start.line - 1 >= m_lineInfo.count() ? m_source.count() - 1 : m_lineInfo.at(tok.start.line - 1);
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

            switch (tok.type) {
            case Whitespace: stream << "Whitespace"; break;
            case Tab: stream << "Tab"; break;
            case Newline: stream << "Newline: " << range << "\n"; continue;
    //        case Tilda: stream << "Tilda"; break;
    //        case Bang: stream << "Bang"; break;
    //        case At: stream << "At"; break;
    //        case Hash: stream << "Hash"; break;
    //        case Dollar: stream << "Dollar"; break;
    //        case Percent: stream << "Percent"; break;
    //        case Cap: stream << "Cap"; break;
    //        case Ampersand: stream << "Ampersand"; break;
    //        case Star: stream << "Star"; break;
            case OpenParenthesis: stream << "OpenParenthesis"; break;
            case CloseParenthesis: stream << "CloseParenthesis"; break;
    //        case Underscore: stream << "Underscore"; break;
    //        case Plus: stream << "Plus"; break;
    //        case OpenCurly: stream << "OpenCurly"; break;
    //        case ClosedCurly: stream << "ClosedCurly"; break;
    //        case Pipe: stream << "Pipe"; break;
            case Colon: stream << "Colon"; break;
    //        case DoubleQuote: stream << "DoubleQuote"; break;
    //        case LessThan: stream << "LessThan"; break;
    //        case GreaterThan: stream << "GreaterThan"; break;
    //        case QuestionMark: stream << "QuestionMark"; break;
    //        case Minus: stream << "Minus"; break;
    //        case Equals: stream << "Equals"; break;
    //        case OpenSquare: stream << "OpenSquare"; break;
    //        case ClosedSquare: stream << "ClosedSquare"; break;
    //        case BackSlash: stream << "BackSlash"; break;
    //        case SemiColon: stream << "SemiColon"; break;
    //        case SingleQuote: stream << "SingleQuote"; break;
    //        case Comma: stream << "Comma"; break;
    //        case Period: stream << "Period"; break;
    //        case Slash: stream << "Slash"; break;
            case Comment: stream << "Comment: " << range << "\n"; continue;
            case False: stream << "False"; break;
            case Point: stream << "Point"; break;
            case Space: stream << "Space"; break;
            case True: stream << "True"; break;
            case Universe: stream << "Universe"; break;
            case Identifier: stream << "Identifier"; break;
            default:
                Q_ASSERT(false); // should never be reached
                break;
            }
            stream << ": " << range << " '" << textForToken(tok) << "'\n";
        }
    }

    void error(const Token& tok, const QString& str, ErrorType type = Error) const
    {
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

        QTextStream out(stderr);
        out << location << '\n' << context << '\n' << caret << '\n';
        if (type == Fatal) {
            out.flush();
            exit(-1);
        }
    }

private:
    QString m_source;
    QString m_name;
    QList<Token> m_tokens;
    QList<int> m_lineInfo;
};

#endif // sourcebuffer_h
