#include "parser.h"

Parser::Parser()
{
    clear();
}

Parser::~Parser()
{
    clear();
}

void Parser::clear()
{
    m_index = -1;
    m_originalSpacesForIndent = 0;
    m_scope = 0;
    m_indent = Unset;
    m_source = 0;
}

void Parser::parse(SourceBuffer* source)
{
    clear();
    m_source = source;

    while (m_index < m_source->tokenCount() - 1) {
        Token tok = advance(1);
        if (tok.type == Newline)
            continue;
        if (tok.type == Alias)
            parseAliasDecl();
        else if (tok.type == Function)
            parseFuncDecl();
        else
            m_source->error(tok, "unexpected token when parsing translation unit");
    }
}

void Parser::newline()
{
}

Token Parser::advance(int i, bool skipComments)
{
    m_index += i;
    if (!skipComments)
        return current();

    while (current().type == Comment)
        m_index++;

    return current();
}

Token Parser::current() const
{
    Q_ASSERT(m_index >= 0);
    Q_ASSERT(m_index < m_source->tokenCount());
    return look(0);
}

Token Parser::look(int i) const
{
    int index = m_index + i;
    Q_ASSERT(m_index >= 0);
    if (index >= m_source->tokenCount())
        return Token();
    return m_source->tokenAt(index);
}

bool Parser::expect(Token tok, TokenType type) const
{
    if (tok.type == type)
        return true;
    m_source->error(tok, "expecting " + typeToString(type) + " for " + m_context);
    return false;
}

bool Parser::checkLeadingWhitespace(const Token& tok)
{
    if (m_indent == Tabs) {
        m_source->error(tok, "unexpected ' ' when already using '\\t' for indentation");
        return false;
    }
    m_indent = Spaces;
    int spacesForIndent = tok.end.column - tok.start.column + 1;
    if (!m_originalSpacesForIndent) {
        m_originalSpacesForIndent = spacesForIndent;
    } else if (spacesForIndent % m_originalSpacesForIndent != 0) {
        m_source->error(tok, "number of spaces in indentation level is not divisable by " + QString::number(m_originalSpacesForIndent));
        return false;
    }
    m_scope = spacesForIndent / m_originalSpacesForIndent;
    return true;
}

bool Parser::checkLeadingTab(const Token& tok)
{
    if (m_indent == Spaces) {
        m_source->error(tok, "unexpected '\\t' when already using ' ' for indentation");
        return false;
    }
    m_indent = Tabs;
    m_scope = tok.end.column - tok.start.column + 1;
    return true;
}

// alias foo bar
void Parser::parseAliasDecl()
{
    m_context = "alias declaration";

    Token tok = advance(1);
    if (!expect(tok, Whitespace))
        return;

    tok = advance(1);
    if (!expect(tok, Identifier))
        return;

    Token type = tok;

    tok = advance(1);
    if (!expect(tok, Whitespace))
        return;

    tok = advance(1);
    if (!expect(tok, Identifier))
        return;

    Token alias = tok;

    tok = advance(1);
    if (!expect(tok, Newline))
        return;

    AliasDecl* decl = new AliasDecl;
    decl->alias = alias;
    decl->type = type;
    m_source->translationUnit()->aliasDecl.append(QSharedPointer<AliasDecl>(decl));
}

// function foo : (foo:Foo?, bar:Bar?, ...)? ->? Baz
void Parser::parseFuncDecl()
{
    m_context = "function declaration";

    Token tok = advance(1);
    if (!expect(tok, Whitespace))
        return;

    tok = advance(1);
    if (!expect(tok, Identifier))
        return;

    Token type = tok;

    tok = advance(1);
    if (!expect(tok, Whitespace))
        return;

    tok = advance(1);
    if (!expect(tok, Colon))
        return;

    QList<FuncDeclArg*> args;
    if (look(1).type == Whitespace && look(2).type == OpenParenthesis)
        args = parseFuncDeclArgs();

    tok = advance(1);
    if (!expect(tok, Minus))
        return;

    tok = advance(1);
    if (!expect(tok, GreaterThan))
        return;

    tok = advance(1);
    if (!expect(tok, Whitespace))
        return;

    tok = advance(1);
    if (!expect(tok, Identifier))
        return;

    Token returnType = type;

    tok = advance(1);
    if (!expect(tok, Newline))
        return;

    FuncStmt* stmt = parseFuncStatement();
    if (!stmt)
        return;

    FuncDecl* decl = new FuncDecl;
    decl->name = type;
    decl->stmt = QSharedPointer<FuncStmt>(stmt);
    decl->returnType = returnType;
    m_source->translationUnit()->funcDecl.append(QSharedPointer<FuncDecl>(decl));
}

QList<FuncDeclArg*> Parser::parseFuncDeclArgs()
{
    QList<FuncDeclArg*> args;
    Token tok = advance(3);
    if (!expect(tok, CloseParenthesis))
        return args;

    tok = advance(1);
    if (!expect(tok, Whitespace))
        return args;

    return args;
}

FuncStmt* Parser::parseFuncStatement()
{
    m_context = "function statement";
    if (!parseIndent(1))
        return 0;

    Expr* expr = parseExpr();
    if (!expr)
        return 0;

    FuncStmt* stmt = new FuncStmt;
    stmt->expr = QSharedPointer<Expr>(expr);
    return stmt;
}

bool Parser::parseIndent(unsigned expected)
{
    Token tok = advance(1);
    if (tok.type == Whitespace && !checkLeadingWhitespace(tok))
        return false;
    else if (tok.type == Tab && !checkLeadingTab(tok))
        return false;

    if (m_scope != expected) {
        m_source->error(tok, "indentation level is incorrect");
        return false;
    }

    return true;
}

Expr* Parser::parseExpr()
{
    m_context = "expression";

    Token tok = advance(1);
    switch (tok.type) {
    case Identifier:
        return parseFuncCallExpr();
    case Digits:
        return parseVarExpr();
    case Return:
        return parseReturnExpr();
    default:
        return 0;
    };
}

VarExpr* Parser::parseVarExpr()
{
    m_context = "variable expression";

    Token digits = current();

    VarExpr* varExpr = new VarExpr;
    varExpr->var = digits;
    return varExpr;
}

ReturnExpr* Parser::parseReturnExpr()
{
    m_context = "return expression";

    Token tok = advance(1);
    if (!expect(tok, Whitespace))
        return 0;

    Expr* expr = parseExpr();
    if (!expr)
        return 0;

    ReturnExpr* returnExpr = new ReturnExpr;
    returnExpr->expr = QSharedPointer<Expr>(expr);
    return returnExpr;
}

FuncCallExpr* Parser::parseFuncCallExpr()
{
    m_context = "function call expression";

    Token callee = current();

    Token tok = advance(1);
    if (!expect(tok, OpenParenthesis))
        return 0;

    QList<QSharedPointer<Expr> > args;
    while(Expr* expr = parseExpr())
        args.append(QSharedPointer<Expr>(expr));

    tok = current();
    if (!expect(tok, CloseParenthesis))
        return 0;

    FuncCallExpr* funcCallExpr = new FuncCallExpr;
    funcCallExpr->callee = callee;
    funcCallExpr->args = args;
    return funcCallExpr;
}
