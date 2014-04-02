#include "lexer.h"

Lexer::Lexer(const QString& text)
    : m_text(text)
{
    clear();
}

Lexer::~Lexer()
{
    clear();
}

void Lexer::clear()
{
    m_index = 0;
    m_column = 0;
    m_tokens.clear();
    m_lineInfo.clear();
}

void Lexer::lex()
{
    clear();
    while (m_index < m_text.size()) {
        const QChar ch = current();
        int index = m_index;
        switch (ch.unicode()) {
        /* whitespace*/
        case ' ': m_tokens.append(Token(Whitespace, index, consumeChar())); break;
        case '\t': m_tokens.append(Token(Tab, index, consumeChar())); break;
        case '\n': m_tokens.append(Token(Newline, index, index)); newline(); break;
        /* punctuators */
//        case '~': m_tokens.append(Token(Tilda, index, index)); break;
//        case '!': m_tokens.append(Token(Bang, index, index)); break;
//        case '@': m_tokens.append(Token(At, index, index)); break;
//        case '#': m_tokens.append(Token(Hash, index, index)); break;
//        case '$': m_tokens.append(Token(Dollar, index, index)); break;
//        case '%': m_tokens.append(Token(Percent, index, index)); break;
//        case '^': m_tokens.append(Token(Cap, index, index)); break;
//        case '&': m_tokens.append(Token(Ampersand, index, index)); break;
//        case '*': m_tokens.append(Token(Star, index, index)); break;
        case '(': m_tokens.append(Token(OpenParenthesis, index, index)); break;
        case ')': m_tokens.append(Token(CloseParenthesis, index, index)); break;
//        case '_': m_tokens.append(Token(Underscore, index, index)); break;
//        case '+': m_tokens.append(Token(Plus, index, index)); break;
//        case '{': m_tokens.append(Token(OpenCurly, index, index)); break;
//        case '}': m_tokens.append(Token(ClosedCurly, index, index)); break;
//        case '|': m_tokens.append(Token(Pipe, index, index)); break;
        case ':': m_tokens.append(Token(Colon, index, index)); break;
//        case '"': m_tokens.append(Token(DoubleQuote, index, index)); break;
//        case '<': m_tokens.append(Token(LessThan, index, index)); break;
//        case '>': m_tokens.append(Token(GreaterThan, index, index)); break;
//        case '?': m_tokens.append(Token(QuestionMark, index, index)); break;
//        case '-': m_tokens.append(Token(Minus, index, index)); break;
//        case '=': m_tokens.append(Token(Equals, index, index)); break;
//        case '[': m_tokens.append(Token(OpenSquare, index, index)); break;
//        case ']': m_tokens.append(Token(ClosedSquare, index, index)); break;
//        case '\\': m_tokens.append(Token(BackSlash, index, index)); break;
//        case ';': m_tokens.append(Token(SemiColon, index, index)); break;
//        case '\'': m_tokens.append(Token(SingleQuote, index, index)); break;
//        case ',': m_tokens.append(Token(Comma, index, index)); break;
//        case '.': m_tokens.append(Token(Period, index, index)); break;
        case '/':
            if (look(1) == '*' && consumeComment()) {
                m_tokens.append(Token(Comment, index, m_index));
                break;
            }
        /*
         * keywords: in alphabetical order
         *  false
         *  point
         *  space
         *  true
         *  universe
         */
        case 'f':
            if (consumeString("alse")) {
                m_tokens.append(Token(False, index, m_index));
                break;
            } else if (consumeIdentifier()) {
                m_tokens.append(Token(Identifier, index, m_index));
                break;
            }
        case 'p':
            if (consumeString("oint")) {
                m_tokens.append(Token(Point, index, m_index));
                break;
            } else if (consumeIdentifier()) {
                m_tokens.append(Token(Identifier, index, m_index));
                break;
            }
        case 's':
            if (consumeString("pace")) {
                m_tokens.append(Token(Space, index, m_index));
                break;
            } else if (consumeIdentifier()) {
                m_tokens.append(Token(Identifier, index, m_index));
                break;
            }
        case 't':
            if (consumeString("rue")) {
                m_tokens.append(Token(True, index, m_index));
                break;
            } else if (consumeIdentifier()) {
                m_tokens.append(Token(Identifier, index, m_index));
                break;
            }
        case 'u':
            if (consumeString("niverse")) {
                m_tokens.append(Token(Universe, index, m_index));
                break;
            } else if (consumeIdentifier()) {
                m_tokens.append(Token(Identifier, index, m_index));
                break;
            }
        /* identifier */
        case '_':
        case 'a': case 'b': case 'c': case 'd': case 'e': /*case 'f':*/
        case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
        case 'm': case 'n': case 'o': /*case 'p':*/ case 'q': case 'r':
        /*case 's': case 't': case 'u':*/ case 'v': case 'w': case 'x':
        case 'y': case 'z':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
        case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
        case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
        case 'Y': case 'Z':
            if (consumeIdentifier()) {
                m_tokens.append(Token(Identifier, index, m_index));
                break;
            }
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        default: m_tokens.append(Token(Undefined, index, index)); break;
        } // end of switch
        advance(1);
    }
}

int Lexer::currentLine() const
{
    return m_lineInfo.count() + 1;
}

int Lexer::currentColumn() const
{
    return m_column;
}

void Lexer::newline()
{
    m_lineInfo.append(m_column);
    m_column = 0;
}

void Lexer::advance(int i)
{
    m_index += i;
    m_column += i;
}

QChar Lexer::current() const
{
    Q_ASSERT(m_index >= 0);
    Q_ASSERT(m_index < m_text.size());
    return look(0);
}

QChar Lexer::look(int i) const
{
    int index = m_index + i;
    if (index < 0 || index >= m_text.size())
        return QChar();
    return m_text.at(index);
}

int Lexer::indexForPosition(int line, int column)
{
    return m_lineInfo.at(line - 1) + column;
}

int Lexer::consumeChar()
{
    const QChar ch = m_text.at(m_index);
    while (m_index + 1 < m_text.size() && m_text.at(m_index + 1) == ch)
        advance(1);
    return m_index;
}

bool Lexer::consumeString(const QString& string)
{
    if (m_text.midRef(m_index + 1, string.length()) == string) {
        advance(string.length());
        return true;
    }
    return false;
}

bool Lexer::consumeComment()
{
    advance(2);
    while (m_index < m_text.size()) {
        advance(1);
        if (look(-1) == '\n')
            newline();
        if (look(-1) == '*' && look(0) == '/')
            break;
    }
    return m_index < m_text.size();
}

bool Lexer::consumeIdentifier()
{
    advance(1);

    static QList<QChar> allowedChars = QList<QChar>()
        << '_'
        << 'a' << 'b' << 'c' << 'd' << 'e' << 'f'
        << 'g' << 'h' << 'i' << 'j' << 'k' << 'l'
        << 'm' << 'n' << 'o' << 'p' << 'q' << 'r'
        << 's' << 't' << 'u' << 'v' << 'w' << 'x'
        << 'y' << 'z'
        << 'A' << 'B' << 'C' << 'D' << 'E' << 'F'
        << 'G' << 'H' << 'I' << 'J' << 'K' << 'L'
        << 'M' << 'N' << 'O' << 'P' << 'Q' << 'R'
        << 'S' << 'T' << 'U' << 'V' << 'W' << 'X'
        << 'Y' << 'Z'
        << '0' << '1' << '2' << '3' << '4'
        << '5' << '6' << '7' << '8' << '9';

    while (m_index < m_text.size()) {
        if (!allowedChars.contains(look(1)))
            break;
        advance(1);
    }
    return m_index < m_text.size();
}

void Lexer::print() const
{
    QTextStream out(stdout);
    print(out);
}

void Lexer::print(QTextStream& stream) const
{
    foreach (Token tok, m_tokens)
        stream << textForToken(tok);
    stream << "\n";
}

void Lexer::printTokens() const
{
    QTextStream out(stdout);
    printTokens(out);
}

void Lexer::printTokens(QTextStream& stream) const
{
    foreach (Token tok, m_tokens) {
        switch (tok.type) {
        case Whitespace: stream << "Whitespace"; break;
        case Tab: stream << "Tab"; break;
        case Newline: stream << "Newline " << tok.start << "," << tok.end << "\n"; continue;
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
        case Comment: stream << "Comment " << tok.start << "," << tok.end << "\n"; continue;
        case False: stream << "False"; break;
        case Point: stream << "Point"; break;
        case Space: stream << "Space"; break;
        case True: stream << "True"; break;
        case Universe: stream << "Universe"; break;
        case Identifier: stream << "Identifier"; break;
        case Undefined: stream << "Undefined"; break;
        default:
            Q_ASSERT(false); // should never be reached
            break;
        }
        stream << ":'" << textForToken(tok) << "' " << tok.start << "," << tok.end << "\n";
    }
}

QString Lexer::textForToken(const Token& tok) const
{
    return m_text.mid(tok.start, tok.end - tok.start + 1);
}
