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
    enum Indent {
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
    void parseFuncDecl();
    QList<QSharedPointer<FuncDeclArg> > parseFuncDeclArgs();
    FuncDeclArg* parseFuncDeclArg();
    FuncStmt* parseFuncStatement();
    bool parseIndent(unsigned expect);
    Expr* parseExpr();
    VarExpr* parseVarExpr();
    ReturnExpr* parseReturnExpr();
    FuncCallExpr* parseFuncCallExpr();

private:
    int m_index;
    int m_originalSpacesForIndent;
    unsigned m_scope;
    Indent m_indent;
    SourceBuffer* m_source;
    QString m_context;
};

#endif // parser_h
