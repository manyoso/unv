#include "lexer.h"

Lexer::Lexer()
    : m_index(-1)
    , m_column(1)
    , m_source(0)
{ }

Lexer::~Lexer()
{
    m_source = 0;
}

void Lexer::lex(SourceBuffer* source)
{
    m_source = source;
    m_index = -1;
    m_column = 0;

    while (m_index < m_source->count() - 1) {
        const QChar ch = advance(1);
        TokenPosition pos = tokenPosition();
        switch (ch.unicode()) {
        /* whitespace*/
        case ' ': appendToken(Whitespace, pos, consumeChar()); break;
        case '\t': appendToken(Tab, pos, consumeChar()); break;
        case '\n': appendToken(Newline, pos, pos); newline(); break;
        /* punctuators */
//        case '~': appendToken(Tilda, pos, pos); break;
//        case '!': appendToken(Bang, pos, pos); break;
//        case '@': appendToken(At, pos, pos); break;
//        case '#': appendToken(Hash, pos, pos); break;
//        case '$': appendToken(Dollar, pos, pos); break;
//        case '%': appendToken(Percent, pos, pos); break;
//        case '^': appendToken(Cap, pos, pos); break;
//        case '&': appendToken(Ampersand, pos, pos); break;
        case '*': appendToken(Star, pos, pos); break;
        case '(': appendToken(OpenParenthesis, pos, pos); break;
        case ')': appendToken(CloseParenthesis, pos, pos); break;
//        case '_': appendToken(Underscore, pos, pos); break;
        case '+': appendToken(Plus, pos, pos); break;
//        case '{': appendToken(OpenCurly, pos, pos); break;
//        case '}': appendToken(ClosedCurly, pos, pos); break;
//        case '|': appendToken(Pipe, pos, pos); break;
        case ':': appendToken(Colon, pos, pos); break;
//        case '"': appendToken(DoubleQuote, pos, pos); break;
        case '<': appendToken(LessThan, pos, pos); break;
        case '>': appendToken(GreaterThan, pos, pos); break;
//        case '?': appendToken(QuestionMark, pos, pos); break;
        case '-':
            if (isDigit(look(1)))
                appendToken(Digits, pos, consumeDigits());
            else
                appendToken(Minus, pos, pos);
            break;
        case '=': appendToken(Equals, pos, pos); break;
//        case '[': appendToken(OpenSquare, pos, pos); break;
//        case ']': appendToken(ClosedSquare, pos, pos); break;
//        case '\\': appendToken(BackSlash, pos, pos); break;
//        case ';': appendToken(SemiColon, pos, pos); break;
//        case '\'': appendToken(SingleQuote, pos, pos); break;
        case ',': appendToken(Comma, pos, pos); break;
        case '.': appendToken(Period, pos, pos); break;
        case '/':
            if (look(1) == '*' && consumeCStyleComment()) {
                appendToken(Comment, pos, tokenPosition());
                break;
            } else if (look(1) == '/' && consumeCPPStyleComment()) {
                appendToken(Comment, pos, tokenPosition());
                break;
            } else {
                appendToken(Slash, pos, pos);
                break;
            }
        /*
         * keywords: in alphabetical order
         */
        case 'e':
            if (consumeString("lse")) {
                appendToken(Else, pos, tokenPosition());
                break;
            } else if (consumeIdentifier()) {
                appendToken(Identifier, pos, tokenPosition());
                break;
            }
        case 'f':
            if (consumeString("alse")) {
                appendToken(False, pos, tokenPosition());
                break;
            } else if (consumeString("unction")) {
                appendToken(Function, pos, tokenPosition());
                break;
            } else if (consumeIdentifier()) {
                appendToken(Identifier, pos, tokenPosition());
                break;
            }
        case 'i':
            if (consumeString("f")) {
                appendToken(If, pos, tokenPosition());
                break;
            } else if (consumeIdentifier()) {
                appendToken(Identifier, pos, tokenPosition());
                break;
            }
        case 'n':
            if (consumeString("ew")) {
                appendToken(New, pos, tokenPosition());
                break;
            } else if (consumeIdentifier()) {
                appendToken(Identifier, pos, tokenPosition());
                break;
            }
        case 'r':
            if (consumeString("eturn")) {
                appendToken(Return, pos, tokenPosition());
                break;
            } else if (consumeIdentifier()) {
                appendToken(Identifier, pos, tokenPosition());
                break;
            }
        case 't':
            if (consumeString("rue")) {
                appendToken(True, pos, tokenPosition());
                break;
            } else if (consumeString("ype")) {
                appendToken(Type, pos, tokenPosition());
                break;
            } else if (consumeIdentifier()) {
                appendToken(Identifier, pos, tokenPosition());
                break;
            }
        /* identifier */
        case '_':
        case 'a': case 'b': case 'c': case 'd': /*case 'e':*/ /*case 'f':*/
        case 'g': case 'h': /*case 'i':*/ case 'j': case 'k': case 'l':
        case 'm': /*case 'n':*/ case 'o': case 'p': case 'q': /*case 'r':*/
        case 's': /*case 't':*/ case 'u': case 'v': case 'w': case 'x':
        case 'y': case 'z':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
        case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
        case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
        case 'Y': case 'Z':
            if (consumeIdentifier()) {
                appendToken(Identifier, pos, tokenPosition());
                break;
            }
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            appendToken(Digits, pos, consumeDigits()); break;
        default:
            m_source->error(createToken(Undefined, pos, pos),
                            "unexpected character when tokenizing file",
                            SourceBuffer::Fatal);
            break;
        } // end of switch
    }
}

void Lexer::newline()
{
    m_source->appendNewline(m_index + 1);
    m_column = 0;
}

QChar Lexer::advance(int i)
{
    m_index += i;
    m_column += i;
    return current();
}

QChar Lexer::current() const
{
    Q_ASSERT(m_index >= 0);
    Q_ASSERT(m_index < m_source->count());
    return look(0);
}

QChar Lexer::look(int i) const
{
    int index = m_index + i;
    Q_ASSERT(index >= 0);
    if (index >= m_source->count())
        return QChar();
    return m_source->at(index);
}

TokenPosition Lexer::tokenPosition() const
{
    TokenPosition pos;
    pos.line = m_source->newlineCount() + 1;
    pos.column = m_column;
    return pos;
}

TokenPosition Lexer::consumeChar()
{
    const QChar ch = m_source->at(m_index);
    while (m_index + 1 < m_source->count() && m_source->at(m_index + 1) == ch)
        advance(1);
    return tokenPosition();
}

bool Lexer::consumeString(const QString& string)
{
    if (m_source->text(m_index + 1, string.length()) == string) {
        advance(string.length());
        return true;
    }
    return false;
}

bool Lexer::consumeCStyleComment()
{
    advance(2);
    while (m_index < m_source->count()) {
        advance(1);
        if (look(-1) == '\n')
            newline();
        if (look(-1) == '*' && look(0) == '/')
            break;
    }
    return m_index < m_source->count();
}

bool Lexer::consumeCPPStyleComment()
{
    advance(1);
    while (m_index < m_source->count()) {
        if (look(1) == '\n')
            break;
        advance(1);
    }
    return m_index < m_source->count();
}

bool Lexer::consumeIdentifier()
{
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

    while (m_index < m_source->count()) {
        if (!allowedChars.contains(look(1)))
            break;
        advance(1);
    }
    return m_index < m_source->count();
}

bool Lexer::isDigit(const QChar& ch) const
{
    static QList<QChar> allowedChars = QList<QChar>()
        << '0' << '1' << '2' << '3' << '4'
        << '5' << '6' << '7' << '8' << '9';
    return allowedChars.contains(ch);
}

TokenPosition Lexer::consumeDigits()
{
    if (current() == '-')
        advance(1);

    while (m_index < m_source->count()) {
        if (!isDigit(look(1)))
            break;
        advance(1);
    }
    return tokenPosition();
}

Token Lexer::createToken(TokenType t, const TokenPosition& s, const TokenPosition& e) const
{
    return Token(t, s, e, m_source->textForTokenPosition(s, e));
}

void Lexer::appendToken(TokenType t, const TokenPosition& s, const TokenPosition& e)
{
    m_source->appendToken(createToken(t, s, e));
}
