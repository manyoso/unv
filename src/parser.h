#ifndef parser_h
#define parser_h

#include <QtCore>

#include "ast.h"
#include "sourcebuffer.h"

class Parser {
public:
    Parser();
    ~Parser();

    void parse(SourceBuffer* source);

private:
    enum Indentation {
        Spaces,
        Tabs,
        Unset
    };

    void clear();
    void newline();
    void advance(int i);
    Token current() const;
    Token look(int) const;

    void parseWhitespace(const Token&);
    void parseTab(const Token&);

private:
    int m_index;
    unsigned m_scope;
    Indentation m_indentation;
    SourceBuffer* m_source;
};

#endif // parser_h
