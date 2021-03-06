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
    m_expectedScope = 0;
    m_indent = Unset;
    m_source = 0;
}

void Parser::parse(SourceBuffer* source)
{
    clear();
    m_source = source;

    QList<Token> currentAttributes;
    while (m_index < m_source->tokenCount() - 1) {
        Token tok = advance(1);
        if (tok.type == Newline)
            continue;
        if (tok.type == Include) {
            parseIncludeDecl();
        } else if (tok.type == Namespace) {
            parseNamespace();
        } else if (tok.type == OpenSquare) {
            currentAttributes = parseTypeAttrs();
        } else if (tok.type == Type) {
            QList<Token> attr = currentAttributes;
            currentAttributes = QList<Token>();
            parseTypeDecl(attr);
        } else if (tok.type == Function) {
            QList<Token> attr = currentAttributes;
            currentAttributes = QList<Token>();
            parseFuncDecl(attr);
        } else {
            m_source->error(tok, "unexpected token when parsing translation unit", SourceBuffer::Fatal);
        }
    }
}

void Parser::newline()
{
}

Token Parser::advance(int i, bool skipComments)
{
    if (m_index + 1 >= m_source->tokenCount()) {
        TokenPosition pos;
        pos.line = m_source->newlineCount() + 1;
        pos.column = 0;
        return Token(EndOfFile, pos, pos, QStringRef());
    }

    if (!skipComments) {
        m_index += i;
        return current(skipComments);
    }

    for (int j = 0; j < i; j++) {
        m_index++;
        while (current(false).type == Comment || (current(false).type == Whitespace && look(1, false).type == Comment))
            m_index++;
    }

    return current();
}

Token Parser::current(bool skipComments) const
{
    assert(m_index >= 0 && m_index < m_source->tokenCount());
    return look(0, skipComments);
}

Token Parser::look(int i, bool skipComments) const
{
    if (!skipComments) {
        int index = m_index + i;
        assert(index >= 0);
        if (index >= m_source->tokenCount())
            return Token();

        return m_source->tokenAt(index);
    }

    int index = m_index;
    for (int j = 0; j < i; j++) {
        index++;
        for (;;) {
            assert(index >= 0);
            if (index >= m_source->tokenCount())
                return Token();

            Token tok = m_source->tokenAt(index);
            Token next = index + 1 >= m_source->tokenCount() ? Token() : m_source->tokenAt(index + 1);
            if (tok.type == Comment || (tok.type == Whitespace && next.type == Comment))
                index++;
            else
                break;
        }
    }

    return m_source->tokenAt(index);
}

bool Parser::expect(Token tok, TokenType type) const
{
    if (tok.type == type)
        return true;
    m_source->error(tok, "expecting " + typeToString(type) + " for " + m_context.top());
    return false;
}

bool Parser::expect(Token tok, const QList<TokenType>& types) const
{
    foreach (TokenType type, types)
        if (tok.type == type)
            return true;

    QStringList typesToString;
    foreach (TokenType type, types)
        typesToString.append(typeToString(type));

    m_source->error(tok, "expecting " + typesToString.join("|") + " for " + m_context.top());
    return false;
}

bool Parser::hasTokenType(const QList<Token>& tokens, TokenType type) const
{
    foreach (Token tok, tokens)
        if (tok.type == type)
            return true;
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

void Parser::parseIncludeDecl()
{
    ParserContext context(this, "include declaration");

    Token tok = advance(1);
    if (!expect(tok, Whitespace))
        return;

    tok = advance(1);
    if (!expect(tok, StringLiteral))
        return;

    IncludeDecl* decl = new IncludeDecl;
    decl->include = tok;

    m_source->translationUnit().includeDecl.append(QSharedPointer<IncludeDecl>(decl));
}

// type foo<T, U>? : (b1:bar(, b2:baz)?)
void Parser::parseTypeDecl(const QList<Token>& attributes)
{
    ParserContext context(this, "type declaration");

    Token tok = advance(1);
    if (!expect(tok, Whitespace))
        return;

    tok = advance(1);
    if (!expect(tok, Identifier))
        return;

    Token name = tok;

    QChar firstChar = name.toStringRef().at(0);
    if (firstChar.toUpper() != firstChar) {
        m_source->error(name, "type names must begin with an upper case char");
        return;
    }

    tok = advance(1);

    QList<QSharedPointer<TypeParam> > params;
    if (tok.type == LessThan) {
        if (!expect(look(1), Identifier))
            return;
        params = parseTypeParams();
        tok = advance(1);
    }

    if (!expect(tok, Whitespace))
        return;

    tok = advance(1);
    if (!expect(tok, Colon))
        return;

    bool isAlias = false;
    QList<QSharedPointer<TypeObject> > objects;
    if (look(1).type == Whitespace && look(2).type == OpenParenthesis) {
        objects = parseTypeObjects();
        tok = advance(1);
    } else {
        isAlias = true;

        tok = advance(1);
        if (!expect(tok, Whitespace))
            return;

        tok = advance(1);
        TypeObject* object = parseTypeObject();
        if (!object) {
            m_source->error(tok, "expected single type for type alias declaration", SourceBuffer::Fatal);
            return;
        }
        objects.append(QSharedPointer<TypeObject>(object));
        tok = current();
    }

    if (!expect(tok, Newline))
        return;

    Node::Kind k = isAlias ? Node::_AliasDecl : Node::_StructDecl;

    TypeDecl* decl = new TypeDecl(k);
    decl->name = name;
    decl->_namespace = m_namespace;
    decl->objects = objects;
    decl->attributes = attributes;

    if (!m_source->typeSystem().addType(*decl))
        return;

    m_source->translationUnit().typeDecl.append(QSharedPointer<TypeDecl>(decl));
}

// function foo<T, U>? : (foo:Foo?, bar:Bar?, ...)? ->? Baz
void Parser::parseFuncDecl(const QList<Token>& attributes)
{
    ParserContext context(this, "function declaration");

    Token tok = advance(1);
    if (!expect(tok, Whitespace))
        return;

    tok = advance(1);
    if (!expect(tok, Identifier))
        return;

    Token name = tok;

    QChar firstChar = name.toStringRef().at(0);
    if (firstChar.toLower() != firstChar) {
        m_source->error(name, "function names must begin with a lower case char");
        return;
    }

    tok = advance(1);

    QList<QSharedPointer<TypeParam> > params;
    if (tok.type == LessThan) {
        if (!expect(look(1), Identifier))
            return;
        params = parseTypeParams();
        tok = advance(1);
    }

    if (!expect(tok, Whitespace))
        return;

    tok = advance(1);
    if (!expect(tok, Colon))
        return;

    QList<QSharedPointer<TypeObject> > objects;
    if (look(1).type == Whitespace && look(2).type == OpenParenthesis)
        objects = parseTypeObjects();

    tok = advance(1);
    if (!expect(tok, Whitespace))
        return;

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

    TypeObject* returnType = parseTypeObject();
    if (!returnType)
        return;

    tok = current();
    if (!expect(tok, Newline))
        return;

    FuncDef* funcDef = 0;
    if (hasTokenType(attributes, Extern)) {
        funcDef = parseEmptyFuncDef();
    } else {
        funcDef = parseFuncDef();
        if (!funcDef)
            return;
    }

    FuncDecl* decl = new FuncDecl;
    decl->name = name;
    decl->_namespace = m_namespace;
    decl->objects = objects;
    decl->funcDef = QSharedPointer<FuncDef>(funcDef);
    decl->returnType = QSharedPointer<TypeObject>(returnType);
    decl->attributes = attributes;

    if (!m_source->typeSystem().addFunction(*decl))
        return;

    m_source->translationUnit().funcDecl.append(QSharedPointer<FuncDecl>(decl));
}

// namespace Name::Space
void Parser::parseNamespace()
{
    ParserContext context(this, "namespace declaration");

    Token tok = advance(1);
    if (!expect(tok, Whitespace))
        return;

    QString _namespace;
    for (;;) {
        tok = advance(1);
        if (!expect(tok, Identifier))
            return;

        _namespace += tok.toString();

        if (look(1).type != Colon)
            break;

        tok = advance(2);
        if (!expect(tok, Colon))
            return;

        _namespace += "::";
    }

    m_namespace = _namespace;
}

// [attr+]\n
QList<Token> Parser::parseTypeAttrs()
{
    ParserContext context(this, "type attribute");

    QList<TokenType> expectedAttributes;
    expectedAttributes.append(Extern);

    Token tok = advance(1);
    if (!expect(tok, expectedAttributes))
        return QList<Token>();

    QList<Token> attributes;
    attributes.append(tok);

    for (;;) {
        if (look(1).type != Comma)
            break;

        Token tok = advance(2);
        if (!expect(tok, Whitespace))
            return QList<Token>();

        tok = advance(1);
        if (!expect(tok, expectedAttributes))
            return QList<Token>();

        attributes.append(tok);
    }

    tok = advance(1);
    if (!expect(tok, ClosedSquare))
        return QList<Token>();

    tok = advance(1);
    if (!expect(tok, Newline))
        return QList<Token>();

    if (look(1).type != Type && look(1).type != Function) {
        m_source->error(tok, "expecting type or function to follow type attribute", SourceBuffer::Fatal);
        return QList<Token>();
    }

    return attributes;
}

QList<QSharedPointer<TypeObject> > Parser::parseTypeObjects()
{
    bool unnamedTypeObject = false;
    QList<QSharedPointer<TypeObject> > objects;
    Token tok = advance(3);
    Token first = tok;
    if (tok.type == Identifier) {
        while(TypeObject* object = parseTypeObject()) {
            if (object->name.type == Undefined)
                unnamedTypeObject = true;
            objects.append(QSharedPointer<TypeObject>(object));
        }
    }

    tok = current();
    if (!expect(tok, CloseParenthesis))
        return QList<QSharedPointer<TypeObject> >();

    if (unnamedTypeObject && objects.size() > 1) {
        m_source->error(first, "a type object list must consist of named objects or only one unnamed object");
        return QList<QSharedPointer<TypeObject> >();
    }

    return objects;
}

TypeObject* Parser::parseTypeObject()
{
    ParserContext context(this, "type object");

    Token tok = current();
    if (tok.type == Comma) {
        tok = advance(1);
        if (!expect(tok, Whitespace))
            return 0;
        tok = advance(1);
    } else if (tok.type == CloseParenthesis)
        return 0;

    if (!expect(tok, Identifier))
        return 0;

    Token identifier1 = tok;
    Token identifier2;

    QList<QSharedPointer<TypeParam> > params;
    if (look(1).type == LessThan) {
        tok = advance(1);
        if (!expect(look(1), Identifier))
            return 0;

        params = parseTypeParams();
    } else if (look(1).type == Colon) {
        tok = advance(2);
        if (!expect(tok, Identifier))
            return 0;

        identifier2 = tok;

        if (look(1).type == LessThan) {
            tok = advance(1);
            if (!expect(look(1), Identifier))
                return 0;

            params = parseTypeParams();
        }
    }

    advance(1);

    TypeObject* arg = new TypeObject;
    arg->name = identifier2.type == Undefined ? Token() : identifier1;
    arg->type = identifier2.type == Identifier ? identifier2 : identifier1;
    return arg;
}

QList<QSharedPointer<TypeParam> > Parser::parseTypeParams()
{
    advance(1); // consume less than
    QList<QSharedPointer<TypeParam> > params;
    while(TypeParam* param = parseTypeParam())
        params.append(QSharedPointer<TypeParam>(param));

    expect(current(1), GreaterThan);
    return params;
}

TypeParam* Parser::parseTypeParam()
{
    Token tok = current();
    if (tok.type == Comma) {
        tok = advance(1);
        if (!expect(tok, Whitespace))
            return 0;
        tok = advance(1);
    } else if (tok.type == GreaterThan)
        return 0;

    if (!expect(tok, Identifier))
        return 0;

    Token identifier = tok;
    advance(1);

    TypeParam* param = new TypeParam;
    param->name = identifier;
    return param;
}

FuncDef* Parser::parseFuncDef()
{
    ParserContext context(this, "function definition");
    IndentLevel indent(this);

    Token tok = current();
    QList<QSharedPointer<Stmt> > stmts;
    while (Stmt* stmt = parseStmt())
        stmts.append(QSharedPointer<Stmt>(stmt));

    if (stmts.isEmpty()) {
        m_source->error(tok, "function must define at least one statement");
        return 0;
    }

    FuncDef* funcDef = new FuncDef;
    funcDef->stmts = stmts;
    return funcDef;
}

FuncDef* Parser::parseEmptyFuncDef()
{
    ParserContext context(this, "function definition");
    IndentLevel indent(this);

    Token tok = current();
    QList<QSharedPointer<Stmt> > stmts;
    while (Stmt* stmt = parseStmt())
        stmts.append(QSharedPointer<Stmt>(stmt));

    if (!stmts.isEmpty())
        m_source->error(tok, "function with extern attribute must not define any statements");
    return 0;
}

bool Parser::parseIndent(unsigned expected)
{
    if (look(1).type != Whitespace && look(1).type != Tab)
        return false;

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

    Expr* lhs = parseBasicExpr();
    if (!lhs)
        return 0;

    return parseBinaryOpExpr(0, lhs);
}

Expr* Parser::parseBasicExpr()
{
    ParserContext context(this, "basic expression");

    Token tok = advance(1);
    Expr* expr = 0;
    switch (tok.type) {
    case Identifier:
        if (look(1).type == OpenParenthesis)
            expr = parseFuncCallExpr();
        else
            expr = parseVarExpr();
        break;
    case BinLiteral:
    case DecLiteral:
    case False:
    case FloatLiteral:
    case HexLiteral:
    case OctLiteral:
    case StringLiteral:
    case True:
        expr = parseLiteralExpr();
        break;
    default:
        break;
    };

    if (expr)
        expr->start = tok;
    return expr;
}

Expr* Parser::parseBinaryOpExpr(int precedence, Expr* lhs)
{
    for (;;) {
        if (look(1).type != Whitespace)
            return lhs;

        int newPrecedence = precedence;
        Token tok = look(2);

        BinaryExpr::BinaryOp op;
        bool foundBinaryOp = false;

        if (tok.type == Equals && look(3).type == Equals
            && precedence <= 1) {
            op = BinaryExpr::OpEquality;
            tok = advance(4);
            foundBinaryOp = true;
        } else if (tok.type == Bang && look(3).type == Equals
            && precedence <= 1) {
            op = BinaryExpr::OpNotEquality;
            tok = advance(4);
            foundBinaryOp = true;
        } else if ((tok.type == LessThan || tok.type == GreaterThan) && look(3).type == Equals
            && precedence <= 1) {
            op = tok.type == LessThan ? BinaryExpr::OpLessThanOrEquality : BinaryExpr::OpGreaterThanOrEquality;
            tok = advance(4);
            foundBinaryOp = true;
        } else if ((tok.type == LessThan || tok.type == GreaterThan)
            && precedence <= 1) {
            op = tok.type == LessThan ? BinaryExpr::OpLessThan : BinaryExpr::OpGreaterThan;
            tok = advance(3);
            foundBinaryOp = true;
        } else if ((tok.type == Plus || tok.type == Minus)
            && precedence <= 2) {
            op = tok.type == Plus ? BinaryExpr::OpAddition : BinaryExpr::OpSubtraction;
            tok = advance(3);
            foundBinaryOp = true;
            newPrecedence = 2;
        } else if ((tok.type == Star || tok.type == Slash)
            && precedence <= 3) {
            op = tok.type == Star ? BinaryExpr::OpMultiplication : BinaryExpr::OpDivision;
            tok = advance(3);
            foundBinaryOp = true;
            newPrecedence = 3;
        }

        if (!foundBinaryOp)
            return lhs;

        if (!expect(tok, Whitespace))
            return 0;

        Expr* rhs = parseBasicExpr();
        if (!rhs)
            return 0;

        Expr* nextRHS = parseBinaryOpExpr(newPrecedence, rhs);
        if (nextRHS)
            rhs = nextRHS;

        BinaryExpr* binaryExpr = new BinaryExpr;
        binaryExpr->op = op;
        binaryExpr->lhs = QSharedPointer<Expr>(lhs);
        binaryExpr->rhs = QSharedPointer<Expr>(rhs);
        binaryExpr->start = lhs->start;
        lhs = binaryExpr;
    }
}

VarExpr* Parser::parseVarExpr()
{
    ParserContext context(this, "variable expression");

    Token var = current();

    VarExpr* varExpr = new VarExpr;
    varExpr->var = var;
    return varExpr;
}

LiteralExpr* Parser::parseLiteralExpr()
{
    ParserContext context(this, "literal expression");

    Token literal = current();

    LiteralExpr* literalExpr = new LiteralExpr;
    literalExpr->literal = literal;
    return literalExpr;
}

TypeCtorExpr* Parser::parseTypeCtorExpr()
{
    ParserContext context(this, "type constructor expression");

    if (look(1).type != New) {
        Expr* expr = parseExpr();
        if (!expr) {
            m_source->error(current(), "expecting a single valid expression to follow '=' without 'new' keyword");
            return 0;
        }

        TypeCtorExpr* typeCtorExpr = new TypeCtorExpr;
        typeCtorExpr->args = QList<QSharedPointer<Expr> >() << QSharedPointer<Expr>(expr);
        return typeCtorExpr;
    }

    Token tok = advance(2);
    if (!expect(tok, Whitespace))
        return 0;

    tok = advance(1);
    if (!expect(tok, Identifier))
        return 0;

    Token type = tok;

    tok = advance(1);
    if (!expect(tok, OpenParenthesis))
        return 0;

    QList<QSharedPointer<Expr> > args;
    while(Expr* expr = parseExpr()) {
        args.append(QSharedPointer<Expr>(expr));

        if (look(1).type == Comma) {
            tok = advance(2);
            if (!expect(tok, Whitespace))
                return 0;
        }
    }

    // This doesn't advance because the last attempt to parse expr in the
    // loop above advanced it for us...
    tok = current();
    if (!expect(tok, CloseParenthesis))
        return 0;

    TypeCtorExpr* typeCtorExpr = new TypeCtorExpr;
    typeCtorExpr->type = type;
    typeCtorExpr->args = args;
    return typeCtorExpr;
}

Stmt* Parser::parseStmt()
{
    ParserContext context(this, "statement");

    while (current().type == Newline && look(1).type == Newline)
        advance(1);

    if (current().type == Newline && !parseIndent(m_expectedScope))
        return 0;

    if (m_index == m_source->tokenCount() - 1)
        return 0;

    Stmt* stmt = 0;
    Token tok = advance(1);
    switch (tok.type) {
    case If:
        stmt = parseIfStmt(); break;
    case Return:
        stmt = parseReturnStmt(); break;
    case Identifier:
        stmt = parseVarDeclStmt(); break;
    default:
        return 0;
    };

    return stmt;
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

    tok = advance(1);

    IndentLevel indent(this);
    Stmt* stmt = parseStmt();
    if (!stmt) {
        m_source->error(tok, "no statement following condition");
        return 0;
    }

    IfStmt* ifStmt = new IfStmt;
    ifStmt->expr = QSharedPointer<Expr>(expr);
    ifStmt->stmt = QSharedPointer<Stmt>(stmt);
    return ifStmt;
}

ReturnStmt* Parser::parseReturnStmt()
{
    ParserContext context(this, "return statement");

    Token keyword = current();

    Token tok = advance(1);
    if (!expect(tok, Whitespace))
        return 0;

    Expr* expr = parseExpr();
    if (!expr)
        return 0;

    if (m_index < m_source->tokenCount() - 1) {
        tok = advance(1);
        if (!expect(tok, Newline))
            return 0;
    }

    ReturnStmt* returnStmt = new ReturnStmt;
    returnStmt->keyword = keyword;
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
    while(Expr* expr = parseExpr()) {
        args.append(QSharedPointer<Expr>(expr));

        if (look(1).type == Comma) {
            tok = advance(2);
            if (!expect(tok, Whitespace))
                return 0;
        }
    }

    // This doesn't advance because the last attempt to parse expr in the
    // loop above advanced it for us...
    tok = current();
    if (!expect(tok, CloseParenthesis))
        return 0;

    FuncCallExpr* funcCallExpr = new FuncCallExpr;
    funcCallExpr->callee = callee;
    funcCallExpr->args = args;
    return funcCallExpr;
}

VarDeclStmt* Parser::parseVarDeclStmt()
{
    ParserContext context(this, "variable declaration statement");

    Token type = current();

    Token tok = advance(1);
    if (!expect(tok, Whitespace))
        return 0;

    tok = advance(1);
    if (!expect(tok, Identifier))
        return 0;

    Token name = tok;

    tok = advance(1);
    if (!expect(tok, Whitespace))
        return 0;

    tok = advance(1);
    if (!expect(tok, Equals))
        return 0;

    tok = advance(1);
    if (!expect(tok, Whitespace))
        return 0;

    TypeCtorExpr* expr = parseTypeCtorExpr();
    if (!expr) {
        m_source->error(tok, "expected expression for variable initialization");
        return 0;
    }

    tok = advance(1);
    if (!expect(tok, Newline))
        return 0;

    VarDeclStmt* varDeclStmt = new VarDeclStmt;
    varDeclStmt->type = type;
    varDeclStmt->name = name;
    varDeclStmt->expr = QSharedPointer<TypeCtorExpr>(expr);
    return varDeclStmt;
}
