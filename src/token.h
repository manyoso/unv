#ifndef token_h
#define token_h

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

struct TokenPosition {
    TokenPosition()
    { line = -1; column = -1; }
    int line;
    int column;
};

struct Token {
    Token()
    { type = Undefined; }
    Token(TokenType t, const TokenPosition& s, const TokenPosition& e)
    { type = t; start = s; end = e; }
    TokenType type;
    TokenPosition start;
    TokenPosition end;
};

#endif // token_h
