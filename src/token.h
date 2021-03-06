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
    Bang,
//    At,
//    Hash,
//    Dollar,
//    Percent,
//    Cap,
//    Ampersand,
    Star,
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
    OpenSquare,
    ClosedSquare,
//    BackSlash,
//    SemiColon,
//    SingleQuote,
    Comma,
    Period,
    Slash,
    /* comment */
    Comment,
    /* keywords */
    Else,
    Extern,
    False,
    Function,
    If,
    Include,
    Namespace,
    New,
    Return,
    True,
    Type,
    /* identifier and literal*/
    Identifier,
    BinLiteral,
    DecLiteral,
    FloatLiteral,
    HexLiteral,
    OctLiteral,
    StringLiteral,
    EndOfFile,
    Undefined
};

static inline QString typeToString(TokenType type)
{
    switch (type) {
    case Whitespace:        return "\' \'";
    case Tab:               return "\'\\t\'";
    case Newline:           return "\'\\n\'";
//    case Tilda:             return "\'~\'";
    case Bang:              return "\'!\'";
//    case At:                return "\'@\'";
//    case Hash:              return "\'#\'";
//    case Dollar:            return "\'$\'";
//    case Percent:           return "\'%\'";
//    case Cap:               return "\'^\'";
//    case Ampersand:         return "\'&\'";
    case Star:              return "\'*\'";
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
    case OpenSquare:        return "\'[\'";
    case ClosedSquare:      return "\']\'";
//    case BackSlash:         return "\'\\\'";
//    case SemiColon:         return "\';\'";
//    case SingleQuote:       return "\'\'\'";
    case Comma:             return "\',\'";
    case Period:            return "\'.\'";
    case Slash:             return "\'/\'";
    case Comment:           return "\'comment\'";
    case Else:              return "\'else\'";
    case Extern:            return "\'extern\'";
    case False:             return "\'false\'";
    case Function:          return "\'function\'";
    case If:                return "\'if\'";
    case Include:           return "\'include'\'";
    case New:               return "\'new\'";
    case Namespace:         return "\'namespace\'";
    case Return:            return "\'return\'";
    case True:              return "\'true\'";
    case Type:              return "\'type\'";
    case Identifier:        return "\'identifier\'";
    case BinLiteral:        return "\'bin literal\'";
    case DecLiteral:        return "\'int literal\'";
    case FloatLiteral:      return "\'float literal\'";
    case HexLiteral:        return "\'hex literal\'";
    case OctLiteral:        return "\'oct literal\'";
    case StringLiteral:     return "\'string literal\'";
    case EndOfFile:         return "\'end of file\'";
    case Undefined:         return "\'undefined\'";
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
    Token(TokenType t, const TokenPosition& s, const TokenPosition& e, const QStringRef& tx)
    { type = t; start = s; end = e; text = tx; }
    TokenType type;
    TokenPosition start;
    TokenPosition end;
    QStringRef text;

    QString toString() const { return text.toString(); }
    QStringRef toStringRef() const { return text; }
};

#endif // token_h
