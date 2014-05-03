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
    enum BinOpPrecedence {
        OpEquality = 1,
        OpLessThanOrEquality = 1,
        OpGreaterThanOrEquality = 1,
        OpLessThan = 1,
        OpGreaterThan = 1,
        OpAddition = 2,
        OpSubtraction = 2,
        OpMultiplication = 3,
        OpDivision = 3
    };

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

    class IndentLevel {
    public:
        IndentLevel(Parser* p)
            : m_p(p)
        {
            m_p->m_expectedScope++;
        }

        ~IndentLevel()
        {
            m_p->m_expectedScope--;
        }

    private:
        Parser* m_p;
    };

    void clear();
    void newline();
    Token advance(int i, bool skipComments = true);
    Token current(bool skipComments = true) const;
    Token look(int i, bool skipComments = true) const;

    bool expect(Token tok, TokenType t) const;
    bool checkLeadingWhitespace(const Token&);
    bool checkLeadingTab(const Token&);

    void parseTypeDecl();
    void parseFuncDecl();
    QList<QSharedPointer<TypeObject> > parseTypeObjects();
    TypeObject* parseTypeObject();
    FuncDef* parseFuncDef();
    bool parseIndent(unsigned expect);
    Expr* parseExpr();
    Expr* parseBasicExpr();
    Expr* parseBinaryOpExpr(int, Expr*);
    VarExpr* parseVarExpr();
    LiteralExpr* parseLiteralExpr();
    TypeCtorExpr* parseTypeCtorExpr();
    Stmt* parseStmt();
    IfStmt* parseIfStmt();
    ReturnStmt* parseReturnStmt();
    FuncCallExpr* parseFuncCallExpr();
    VarDeclStmt* parseVarDeclStmt();

private:
    int m_index;
    int m_originalSpacesForIndent;
    unsigned m_scope;
    unsigned m_expectedScope;
    Indent m_indent;
    SourceBuffer* m_source;
    QStack<QString> m_context;
};

#endif // parser_h
