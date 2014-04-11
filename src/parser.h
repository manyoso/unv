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

    class ParserContext {
    public:
        ParserContext(Parser* p, const QString& context)
            : m_p(p)
        {
            m_p->m_context.push(context);
        }
        ~ParserContext()
        {
            m_p->m_context.pop();
        }
    private:
        Parser* m_p;
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
    FuncStmt* parseFuncStmt();
    bool parseIndent(unsigned expect);
    Expr* parseExpr();
    VarExpr* parseVarExpr();
    ExprStmt* parseExprStmt();
    IfStmt* parseIfStmt();
    ReturnStmt* parseReturnStmt();
    FuncCallExpr* parseFuncCallExpr();

private:
    int m_index;
    int m_originalSpacesForIndent;
    unsigned m_scope;
    Indent m_indent;
    SourceBuffer* m_source;
    QStack<QString> m_context;
};

#endif // parser_h
