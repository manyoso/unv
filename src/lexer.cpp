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
    m_column = 1;

    while (m_index < m_source->count() - 1) {
        advance(1);
        const QChar ch = current();
        TokenPosition pos = tokenPosition();
        switch (ch.unicode()) {
        /* whitespace*/
        case ' ': m_source->appendToken(Token(Whitespace, pos, consumeChar())); break;
        case '\t': m_source->appendToken(Token(Tab, pos, consumeChar())); break;
        case '\n': m_source->appendToken(Token(Newline, pos, pos)); newline(); break;
        /* punctuators */
//        case '~': m_source->appendToken(Token(Tilda, pos, pos)); break;
//        case '!': m_source->appendToken(Token(Bang, pos, pos)); break;
//        case '@': m_source->appendToken(Token(At, pos, pos)); break;
//        case '#': m_source->appendToken(Token(Hash, pos, pos)); break;
//        case '$': m_source->appendToken(Token(Dollar, pos, pos)); break;
//        case '%': m_source->appendToken(Token(Percent, pos, pos)); break;
//        case '^': m_source->appendToken(Token(Cap, pos, pos)); break;
//        case '&': m_source->appendToken(Token(Ampersand, pos, pos)); break;
//        case '*': m_source->appendToken(Token(Star, pos, pos)); break;
        case '(': m_source->appendToken(Token(OpenParenthesis, pos, pos)); break;
        case ')': m_source->appendToken(Token(CloseParenthesis, pos, pos)); break;
//        case '_': m_source->appendToken(Token(Underscore, pos, pos)); break;
        case '+': m_source->appendToken(Token(Plus, pos, pos)); break;
//        case '{': m_source->appendToken(Token(OpenCurly, pos, pos)); break;
//        case '}': m_source->appendToken(Token(ClosedCurly, pos, pos)); break;
//        case '|': m_source->appendToken(Token(Pipe, pos, pos)); break;
        case ':': m_source->appendToken(Token(Colon, pos, pos)); break;
//        case '"': m_source->appendToken(Token(DoubleQuote, pos, pos)); break;
        case '<': m_source->appendToken(Token(LessThan, pos, pos)); break;
        case '>': m_source->appendToken(Token(GreaterThan, pos, pos)); break;
//        case '?': m_source->appendToken(Token(QuestionMark, pos, pos)); break;
        case '-': m_source->appendToken(Token(Minus, pos, pos)); break;
        case '=': m_source->appendToken(Token(Equals, pos, pos)); break;
//        case '[': m_source->appendToken(Token(OpenSquare, pos, pos)); break;
//        case ']': m_source->appendToken(Token(ClosedSquare, pos, pos)); break;
//        case '\\': m_source->appendToken(Token(BackSlash, pos, pos)); break;
//        case ';': m_source->appendToken(Token(SemiColon, pos, pos)); break;
//        case '\'': m_source->appendToken(Token(SingleQuote, pos, pos)); break;
        case ',': m_source->appendToken(Token(Comma, pos, pos)); break;
        case '.': m_source->appendToken(Token(Period, pos, pos)); break;
        case '/':
            if (look(1) == '*' && consumeComment()) {
                m_source->appendToken(Token(Comment, pos, tokenPosition()));
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
        case 'a':
            if (consumeString("lias")) {
                m_source->appendToken(Token(Alias, pos, tokenPosition()));
                break;
            } else if (consumeString("pply")) {
                m_source->appendToken(Token(Apply, pos, tokenPosition()));
                break;
            } else if (consumeIdentifier()) {
                m_source->appendToken(Token(Identifier, pos, tokenPosition()));
                break;
            }
        case 'c':
            if (consumeString("onstruct")) {
                m_source->appendToken(Token(Construct, pos, tokenPosition()));
                break;
            } else if (consumeIdentifier()) {
                m_source->appendToken(Token(Identifier, pos, tokenPosition()));
                break;
            }
        case 'f':
            if (consumeString("alse")) {
                m_source->appendToken(Token(False, pos, tokenPosition()));
                break;
            } else if (consumeIdentifier()) {
                m_source->appendToken(Token(Identifier, pos, tokenPosition()));
                break;
            }
        case 'r':
            if (consumeString("eturn")) {
                m_source->appendToken(Token(Return, pos, tokenPosition()));
                break;
            } else if (consumeIdentifier()) {
                m_source->appendToken(Token(Identifier, pos, tokenPosition()));
                break;
            }
        case 't':
            if (consumeString("rue")) {
                m_source->appendToken(Token(True, pos, tokenPosition()));
                break;
            } else if (consumeString("ype")) {
                m_source->appendToken(Token(Type, pos, tokenPosition()));
                break;
            } else if (consumeIdentifier()) {
                m_source->appendToken(Token(Identifier, pos, tokenPosition()));
                break;
            }
        /* identifier */
        case '_':
        /*case 'a':*/ case 'b': /*case 'c':*/ case 'd': case 'e': /*case 'f':*/
        case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
        case 'm': case 'n': case 'o': /*case 'p':*/ case 'q': /*case 'r':*/
        /*case 's': case 't': case 'u':*/ case 'v': case 'w': case 'x':
        case 'y': case 'z':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
        case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
        case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
        case 'Y': case 'Z':
            if (consumeIdentifier()) {
                m_source->appendToken(Token(Identifier, pos, tokenPosition()));
                break;
            }
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            m_source->appendToken(Token(Digits, pos, consumeDigits())); break;
        default:
            m_source->error(Token(Undefined, pos, pos),
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

void Lexer::advance(int i)
{
    m_index += i;
    m_column += i;
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

bool Lexer::consumeComment()
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

TokenPosition Lexer::consumeDigits()
{
    static QList<QChar> allowedChars = QList<QChar>()
        << '0' << '1' << '2' << '3' << '4'
        << '5' << '6' << '7' << '8' << '9';
    while (m_index < m_source->count()) {
        if (!allowedChars.contains(look(1)))
            break;
        advance(1);
    }
    return tokenPosition();
}

