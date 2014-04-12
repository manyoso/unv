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
    Plus,
//    OpenCurly,
//    ClosedCurly,
//    Pipe,
    Colon,
//    DoubleQuote,
    LessThan,
    GreaterThan,
//    QuestionMark,
    Minus,
    Equals,
//    OpenSquare,
//    ClosedSquare,
//    BackSlash,
//    SemiColon,
//    SingleQuote,
    Comma,
    Period,
    Slash,
    /* comment */
    Comment,
    /* keywords */
    Alias,
    Else,
    False,
    Function,
    If,
    Return,
    True,
    Type,
    /* identifier */
    Identifier,
    Digits,
    Undefined
};

static inline QString typeToString(TokenType type)
{
    switch (type) {
    case Whitespace:        return "\' \'";
    case Tab:               return "\'\\t\'";
    case Newline:           return "\'\\n\'";
//    case Tilda:             return "\'~\'";
//    case Bang:              return "\'!\'";
//    case At:                return "\'@\'";
//    case Hash:              return "\'#\'";
//    case Dollar:            return "\'$\'";
//    case Percent:           return "\'%\'";
//    case Cap:               return "\'^\'";
//    case Ampersand:         return "\'&\'";
//    case Star:              return "\'*\'";
    case OpenParenthesis:   return "\'(\'";
    case CloseParenthesis:  return "\')\'";
//    case Underscore:        return "\'_\'";
    case Plus:              return "\'+\'";
//    case OpenCurly:         return "\'{\'";
//    case ClosedCurly:       return "\'}\'";
//    case Pipe:              return "\'|\'";
    case Colon:             return "\':\'";
//    case DoubleQuote:       return "\'\"\'";
    case LessThan:          return "\'<\'";
    case GreaterThan:       return "\'<\'";
//    case QuestionMark:      return "\'?\'";
    case Minus:             return "\'-\'";
    case Equals:            return "\'=\'";
//    case OpenSquare:        return "\'[\'";
//    case ClosedSquare:      return "\']\'";
//    case BackSlash:         return "\'\\\'";
//    case SemiColon:         return "\';\'";
//    case SingleQuote:       return "\'\'\'";
    case Comma:             return "\',\'";
    case Period:            return "\'.\'";
//    case Slash:             return "\'/\'";
    case Comment:           return "\'comment\'";
    case Alias:             return "\'alias\'";
    case Else:              return "\'else\'";
    case False:             return "\'false\'";
    case Function:          return "\'function\'";
    case If:                return "\'if\'";
    case Return:            return "\'return\'";
    case True:              return "\'true\'";
    case Type:              return "\'type\'";
    case Identifier:        return "\'identifier\'";
    case Digits:            return "\'digits\'";
    default:
        Q_ASSERT(false); // should never be reached
        break;
    }
}

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
