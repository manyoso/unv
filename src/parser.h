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
    Token advance(int i, bool skipComments = true);
    Token current() const;
    Token look(int) const;

    bool expect(Token tok, TokenType t) const;
    bool checkLeadingWhitespace(const Token&);
    bool checkLeadingTab(const Token&);

    void parseAliasDecl();
    void parseTypeDecl();
    QList<TypeDeclArg*> parseTypeDeclArgs();
    void parseTypeStatement();
    void parseIndentation();

private:
    int m_index;
    int m_spacesForIndent;
    unsigned m_scope;
    Indentation m_indentation;
    SourceBuffer* m_source;
    QString m_context;
};

#endif // parser_h
