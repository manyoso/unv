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
    void advance(int i);
    QChar current() const;
    QChar look(int) const;
    TokenPosition tokenPosition() const;
    TokenPosition consumeChar();
    bool consumeString(const QString&);
    bool consumeComment();
    bool consumeIdentifier();
    TokenPosition consumeDigits();

private:
    int m_index;
    int m_column;
    SourceBuffer* m_source;
};

#endif // lexer_h
