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

    while (current().type == Comment || (current().type == Whitespace && look(1).type == Comment))
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
    m_source->error(tok, "expecting " + typeToString(type) + " for " + m_context.top());
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
    ParserContext context(this, "alias declaration");

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
    ParserContext context(this, "function declaration");

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

    QList<QSharedPointer<FuncDeclArg> > args;
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

    FuncStmt* stmt = parseFuncStmt();
    if (!stmt)
        return;

    FuncDecl* decl = new FuncDecl;
    decl->name = type;
    decl->stmt = QSharedPointer<FuncStmt>(stmt);
    decl->returnType = returnType;
    m_source->translationUnit()->funcDecl.append(QSharedPointer<FuncDecl>(decl));
}

QList<QSharedPointer<FuncDeclArg> > Parser::parseFuncDeclArgs()
{
    QList<QSharedPointer<FuncDeclArg> > args;
    Token tok = advance(3);
    if (tok.type == Identifier) {
        while(FuncDeclArg* arg = parseFuncDeclArg())
            args.append(QSharedPointer<FuncDeclArg>(arg));
    }

    tok = current();
    if (!expect(tok, CloseParenthesis))
        return args;

    tok = advance(1);
    if (!expect(tok, Whitespace))
        return args;

    return args;
}

FuncDeclArg* Parser::parseFuncDeclArg()
{
    ParserContext context(this, "function declaration argument");

    Token tok = current();
    if (tok.type == Comma) {
        tok = advance(1);
        if (!expect(tok, Whitespace))
            return 0;
    } else if (tok.type == CloseParenthesis)
        return 0;

    if (!expect(tok, Identifier))
        return 0;

    Token name = tok;

    tok = advance(1);
    if (!expect(tok, Colon))
        return 0;

    tok = advance(1);
    if (!expect(tok, Identifier))
        return 0;

    advance(1);

    Token type = tok;
    FuncDeclArg* arg = new FuncDeclArg;
    arg->name = name;
    arg->type = type;
    return arg;
}

FuncStmt* Parser::parseFuncStmt()
{
    ParserContext context(this, "function statement");
    if (!parseIndent(1))
        return 0;

    QList<QSharedPointer<ExprStmt> > stmts;
    while (ExprStmt* exprStmt = parseExprStmt())
        stmts.append(QSharedPointer<ExprStmt>(exprStmt));

    FuncStmt* stmt = new FuncStmt;
    stmt->stmts = stmts;
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
    ParserContext context(this, "expression");

    Token tok = advance(1);
    switch (tok.type) {
    case Identifier:
        if (look(1).type == OpenParenthesis)
            return parseFuncCallExpr();
        else
            return parseVarExpr();
    case Digits:
        return parseVarExpr();
    default:
        return 0;
    };
}

VarExpr* Parser::parseVarExpr()
{
    ParserContext context(this, "variable expression");

    Token var = current();

    VarExpr* varExpr = new VarExpr;
    varExpr->var = var;
    return varExpr;
}

ExprStmt* Parser::parseExprStmt()
{
    ParserContext context(this, "expression statement");
    Token tok = advance(1);
    switch (tok.type) {
    case If:
        return parseIfStmt();
    case Return:
        return parseReturnStmt();
    default:
        return 0;
    };
}

IfStmt* Parser::parseIfStmt()
{
    ParserContext context(this, "if statement");

    Token tok = advance(1);
    if (!expect(tok, Whitespace))
        return 0;

    tok = advance(1);
    if (!expect(tok, OpenParenthesis))
        return 0;

    Expr* expr = parseExpr();
    if (!expr)
        return 0;

    tok = advance(1);
    if (!expect(tok, CloseParenthesis))
        return 0;

    ExprStmt* exprStmt = parseExprStmt();
    if (!exprStmt)
        return 0;

    IfStmt* ifStmt = new IfStmt;
    ifStmt->expr = QSharedPointer<Expr>(expr);
    ifStmt->exprStmt = QSharedPointer<ExprStmt>(exprStmt);
    return ifStmt;
}

ReturnStmt* Parser::parseReturnStmt()
{
    ParserContext context(this, "return statement");

    Token tok = advance(1);
    if (!expect(tok, Whitespace))
        return 0;

    Expr* expr = parseExpr();
    if (!expr)
        return 0;

    ReturnStmt* returnStmt = new ReturnStmt;
    returnStmt->expr = QSharedPointer<Expr>(expr);
    return returnStmt;
}

FuncCallExpr* Parser::parseFuncCallExpr()
{
    ParserContext context(this, "function call expression");

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
