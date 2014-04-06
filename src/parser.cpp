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
        const Token& tok = current();

        switch (tok.type) {
        case Comment:
            continue;
        case Whitespace:
            parseWhitespace(tok); break;
        case Tab:
            parseTab(tok); break;
        case Newline:
        {
            const Token& next = look(1);
            switch (next.type) {
            case Whitespace:
                parseWhitespace(next); break;
            case Tab:
                parseTab(next); break;
            default: break;
            }
            break;
        }
        default:
            error(tok, "unexpected token when parsing translation unit"); break;
        }
    }
}

void Parser::newline()
{
}

void Parser::advance(int i)
{
    m_index += i;
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

void Parser::error(const Token& tok, const QString& str) const
{
    QString location = m_source->name()
        + ":" + QString::number(tok.start.line)
        + ":" + QString::number(tok.start.column)
        + " error: " + str;
    QString context = m_source->lineForToken(tok);
    QString caret(tok.start.column - 1, ' ');
    caret += QString(tok.end.column - tok.start.column + 1, '^');

    // TODO: Print in color to the output
    QTextStream out(stderr);
    out << location << '\n' << context << '\n' << caret << '\n';
}

void Parser::parseWhitespace(const Token& tok)
{
    if (m_indentation == Unset)
        m_indentation = Spaces;
    else if (m_indentation == Tabs)
        error(tok, "unexpected ' ' when already using '\\t' for indentation");
}

void Parser::parseTab(const Token& tok)
{
    if (m_indentation == Unset)
        m_indentation = Tabs;
    else if (m_indentation == Spaces)
        error(tok, "unexpected '\\t' when already using ' ' for indentation");
}
