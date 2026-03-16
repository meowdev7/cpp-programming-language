#include "parser.h"
#include "error.h"
#include <iostream>

Token current(Parser &p)
{
    if (p.position >= p.tokens.size())
        return p.tokens.back();
    return p.tokens[p.position];
}

Token advance(Parser &p)
{
    if (p.position < p.tokens.size())
        p.position++;
    return current(p);
}

bool match(Parser &p, TokenType type)
{
    if (current(p).type == type)
    {
        advance(p);
        return true;
    }
    return false;
}

Token consume(Parser& p, TokenType type, const std::string& msg)
{
    if (current(p).type == type)
    {
        Token token = current(p);
        advance(p);
        return token;
    }
    error(current(p).line, current(p).column, msg);
    exit(1);
}

bool isOperator(TokenType type)
{
    return type == TokenType::Plus || type == TokenType::Minus ||
           type == TokenType::Star || type == TokenType::Slash ||
           type == TokenType::EqualEqual || type == TokenType::BangEqual ||
           type == TokenType::Greater || type == TokenType::Less ||
           type == TokenType::GreaterEqual || type == TokenType::LessEqual;
}
