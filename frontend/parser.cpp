#include "parser.h"
#include <iostream>

Token current(Parser &p)
{
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

    std::cerr << "Unknown statement\n";
    return nullptr;
}

Statement* parseVariableDeclaration(Parser &p)
{
    Token typeToken = current(p);
    advance(p);

    Token nameToken = current(p);

    if (nameToken.type != TokenType::Identifier)
    {
        std::cerr << "Expected identifier\n";
        return nullptr;
    }

    advance(p);

    if (!match(p, TokenType::Equal))
    {
        std::cerr << "Expected '=' after variable name\n";
        return nullptr;
    }

    Expression* value = parseExpression(p);

    if (!match(p, TokenType::Semicolon))
        std::cerr << "Expected ';'\n";

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
        Token op = current(p);
        advance(p);

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

    std::cerr << "Unexpected token in expression\n";
    return nullptr;
}