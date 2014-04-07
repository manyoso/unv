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
    m_scope = 0;
    m_indentation = Unset;
    m_source = 0;
}

void Parser::parse(SourceBuffer* source)
{
    clear();
    m_source = source;

    while (m_index < m_source->tokenCount() - 1) {
        advance(1);
        Token tok = current();
        if (tok.type == Newline)
            continue;
        if (tok.type == Alias)
            parseAliasDecl();
        else if (tok.type == Type)
            parseTypeDecl();
        else
            m_source->error(tok, "unexpected token when parsing translation unit");
#if 0
        const Token& tok = current();

        switch (tok.type) {
        case Whitespace:
            parseLeadingWhitespace(tok); break;
        case Tab:
            parseLeadingTab(tok); break;
        case Newline:
        {
            const Token& next = look(1);
            switch (next.type) {
            case Whitespace:
                parseLeadingWhitespace(next); break;
            case Tab:
                parseLeadingTab(next); break;
            default: break;
            }
            break;
        }
        default:
            m_source->error(tok, "unexpected token when parsing translation unit"); break;
        }
#endif
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

bool Parser::expect(Token tok, TokenType type)
{
    if (tok.type == type)
        return true;
    m_source->error(tok, "expecting " + typeToString(type) + " for " + m_context);
    return false;
}

void Parser::parseLeadingWhitespace(const Token& tok)
{
    if (m_indentation == Unset)
        m_indentation = Spaces;
    else if (m_indentation == Tabs)
        m_source->error(tok, "unexpected ' ' when already using '\\t' for indentation");
}

void Parser::parseLeadingTab(const Token& tok)
{
    if (m_indentation == Unset)
        m_indentation = Tabs;
    else if (m_indentation == Spaces)
        m_source->error(tok, "unexpected '\\t' when already using ' ' for indentation");
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
    m_source->translationUnit()->aliasDecl.append(decl);
}

// type foo : (foo:Foo?, bar:Bar?, ...)? ->? Baz
void Parser::parseTypeDecl()
{
    m_context = "type declaration";

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

    tok = advance(1);
    if (!expect(tok, Whitespace))
        return;

    tok = advance(1);
    if (!expect(tok, OpenParenthesis))
        return;

    tok = advance(1);
    if (!expect(tok, CloseParenthesis))
        return;

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
    if (!expect(tok, Identifier))
        return;

    Token returnType = type;

    tok = advance(1);
    if (!expect(tok, Newline))
        return;

    TypeDecl* decl = new TypeDecl;
    decl->type = type;
    decl->returnType = returnType;
    m_source->translationUnit()->typeDecl.append(decl);
}
