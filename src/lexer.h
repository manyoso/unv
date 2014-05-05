#ifndef lexer_h
#define lexer_h

#include <QtCore>

#include "sourcebuffer.h"

class Lexer {
public:
    Lexer();
    ~Lexer();

    void lex(SourceBuffer* source);

private:
    void newline();
    QChar advance(int i);
    QChar current() const;
    QChar look(int) const;
    TokenPosition tokenPosition() const;
    TokenPosition consumeChar();
    bool consumeString(const QString&);
    bool consumeCStyleComment();
    bool consumeCPPStyleComment();
    bool consumeIdentifier();
    bool consumeStringLiteral();

    bool isNumericLiteral() const;
    void handleNumericLiteral();
    void handleOctalOrFloatLiteral(const TokenPosition&);
    void handleDecimalOrFloatLiteral(const TokenPosition&);

    bool isDigit(const QChar&) const;
    bool isHexDigit(const QChar&) const;
    bool isBinDigit(const QChar&) const;
    bool isOctDigit(const QChar&) const;
    TokenPosition consumeHexLiteral();
    TokenPosition consumeBinLiteral();
    TokenPosition consumeOctLiteral();
    TokenPosition consumeFloatLiteral();
    TokenPosition consumeDecLiteral();
    Token createToken(TokenType t, const TokenPosition& s, const TokenPosition& e) const;
    void appendToken(TokenType t, const TokenPosition& s, const TokenPosition& e);

private:
    int m_index;
    int m_column;
    SourceBuffer* m_source;
};

#endif // lexer_h
