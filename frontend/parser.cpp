#include "parser.h"
#include "error.h"
#include <iostream>

Token current(Parser &p)
{
    // If we've gone past the end of the token list, return the last token (which should be EndOfFile)
    
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
        return advance(p);

    error(current(p).line, current(p).column, msg);
    exit(1);
}

bool isOperator(TokenType type)
{
    return type == TokenType::Plus ||
           type == TokenType::Minus ||
           type == TokenType::Star ||
           type == TokenType::Slash;
}

Program parseProgram(Parser &p)
{
    Program program;

    while (current(p).type != TokenType::EndOfFile)
    {
        Statement* stmt = parseStatement(p);

        if (stmt)
            program.statements.emplace_back(stmt);
    }

    return program;
}

Statement* parseStatement(Parser &p)
{
    Token t = current(p);

    if (t.type == TokenType::TypeInt ||
        t.type == TokenType::TypeFloat ||
        t.type == TokenType::TypeString ||
        t.type == TokenType::TypeBool)
    {
        return parseVariableDeclaration(p);
    }

    error(t.line, t.column, "Unknown statement");
    return nullptr;
}

Statement* parseVariableDeclaration(Parser &p)
{
    Token typeToken = advance(p);

    Token nameToken = consume(
        p,
        TokenType::Identifier,
        "Expected variable name after type"
    );

    consume(
        p,
        TokenType::Equal,
        "Expected '=' after variable name"
    );

    Expression* value = parseExpression(p);

    consume(
        p,
        TokenType::Semicolon,
        "Expected ';' after declaration"
    );

    auto* node = new VariableDeclaration();
    node->type = typeToken.value;
    node->name = nameToken.value;
    node->value.reset(value);

    return node;
}

Expression* parseExpression(Parser &p)
{
    return parseBinaryExpression(p);
}

Expression* parseBinaryExpression(Parser &p)
{
    Expression* left = parsePrimary(p);

    while (isOperator(current(p).type))
    {
        Token op = advance(p);

        Expression* right = parsePrimary(p);

        auto* expr = new BinaryExpression();
        expr->left.reset(left);
        expr->op = op.value;
        expr->right.reset(right);

        left = expr;
    }

    return left;
}

Expression* parsePrimary(Parser &p)
{
    Token t = current(p);

    if (t.type == TokenType::Number)
    {
        advance(p);

        auto* node = new NumberLiteral();
        node->value = t.value;
        return node;
    }

    if (t.type == TokenType::String)
    {
        advance(p);

        auto* node = new StringLiteral();
        node->value = t.value;
        return node;
    }

    if (t.type == TokenType::Identifier)
    {
        advance(p);

        auto* node = new Identifier();
        node->name = t.value;
        return node;
    }

    error(t.line, t.column, "Unexpected token in expression");
    exit(1);
}