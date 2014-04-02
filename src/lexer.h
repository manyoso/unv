#ifndef lexer_h
#define lexer_h

#include <QtCore>

enum TokenType {
    /* whitespace */
    Whitespace,
    Tab,
    Newline,
    /* punctuators */
//    Tilda,
//    Bang,
//    At,
//    Hash,
//    Dollar,
//    Percent,
//    Cap,
//    Ampersand,
//    Star,
    OpenParenthesis,
    CloseParenthesis,
//    Underscore,
//    Plus,
//    OpenCurly,
//    ClosedCurly,
//    Pipe,
    Colon,
//    DoubleQuote,
//    LessThan,
//    GreaterThan,
//    QuestionMark,
//    Minus,
//    Equals,
//    OpenSquare,
//    ClosedSquare,
//    BackSlash,
//    SemiColon,
//    SingleQuote,
//    Comma,
//    Period,
    Slash,
    /* comment */
    Comment,
    /* keywords */
    False,
    Point,
    Space,
    True,
    Universe,
    /* identifier */
    Identifier,
    Undefined
};

struct Token {
    Token(TokenType t, unsigned s, unsigned e)
    { type = t; start = s; end = e; }
    TokenType type;
    unsigned start;
    unsigned end;
};

class Lexer {
public:
    Lexer(const QString& text);
    ~Lexer();

    void lex();
    void print() const;
    void print(QTextStream&) const;
    void printTokens() const;
    void printTokens(QTextStream&) const;

private:
    void clear();
    int currentLine() const;
    int currentColumn() const;
    void newline();
    void advance(int i);
    QChar current() const;
    QChar look(int) const;
    int indexForPosition(int line, int column);
    int consumeChar();
    bool consumeString(const QString&);
    bool consumeComment();
    bool consumeIdentifier();
    QString textForToken(const Token& tok) const;

private:
    QString m_text;
    int m_index;
    int m_column;
    QList<Token> m_tokens;
    QList<int> m_lineInfo;
};

#endif // lexer_h
