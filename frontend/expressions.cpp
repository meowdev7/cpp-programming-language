#include "parser.h"
#include "error.h"
#include <iostream>

Expression* parseComparison(Parser &p);
Expression* parseTerm(Parser &p);
Expression* parseFactor(Parser &p);

Expression* parseExpression(Parser &p)
{
    return parseBinaryExpression(p);
}

Expression* parseBinaryExpression(Parser &p)
{
    return parseComparison(p);
}

Expression* parseComparison(Parser &p)
{
    Expression* left = parseTerm(p);

    while (current(p).type == TokenType::EqualEqual ||
           current(p).type == TokenType::BangEqual ||
           current(p).type == TokenType::Greater ||
           current(p).type == TokenType::Less ||
           current(p).type == TokenType::GreaterEqual ||
           current(p).type == TokenType::LessEqual)
    {
        Token op = current(p);
        advance(p);
        Expression* right = parseTerm(p);

        auto* expr = new BinaryExpression();
        expr->left.reset(left);
        expr->op = op.value;
        expr->right.reset(right);
        left = expr;
    }
    return left;
}

Expression* parseTerm(Parser &p)
{
    Expression* left = parseFactor(p);

    while (current(p).type == TokenType::Plus ||
           current(p).type == TokenType::Minus)
    {
        Token op = current(p);
        advance(p);
        Expression* right = parseFactor(p);

        auto* expr = new BinaryExpression();
        expr->left.reset(left);
        expr->op = op.value;
        expr->right.reset(right);
        left = expr;
    }
    return left;
}

Expression* parseFactor(Parser &p)
{
    if (current(p).type == TokenType::Minus)
    {
        advance(p);
        Expression* operand = parsePrimary(p);
        
        auto* expr = new BinaryExpression();
        auto* zero = new NumberLiteral();
        zero->value = "0";
        expr->left.reset(zero);
        expr->op = "-";
        expr->right.reset(operand);
        return expr;
    }
    
    Expression* left = parsePrimary(p);

    while (current(p).type == TokenType::Star ||
           current(p).type == TokenType::Slash)
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

    if (t.type == TokenType::True || t.type == TokenType::False)
    {
        advance(p);
        auto* node = new BooleanLiteral();
        node->value = (t.type == TokenType::True);
        return node;
    }

    if (t.type == TokenType::Identifier)
    {
        advance(p);

        if (current(p).type == TokenType::LeftBracket)
            return parseIndexExpression(p, t.value);

        if (current(p).type == TokenType::LeftParen)
            return parseCallExpression(p, t.value);

        auto* node = new Identifier();
        node->name = t.value;
        return node;
    }

    if (t.type == TokenType::LeftBracket)
        return parseArrayLiteral(p);

    error(t.line, t.column, "Unexpected token in expression");
    exit(1);
}

Expression* parseCallExpression(Parser &p, const std::string& funcName)
{
    consume(p, TokenType::LeftParen, "Expected '(' in function call");
    
    std::vector<std::unique_ptr<Expression>> args;
    
    if (current(p).type != TokenType::RightParen)
    {
        Expression* arg = parseExpression(p);
        args.emplace_back(arg);
        
        while (current(p).type == TokenType::Comma)
        {
            advance(p);
            arg = parseExpression(p);
            args.emplace_back(arg);
        }
    }
    
    consume(p, TokenType::RightParen, "Expected ')' after arguments");
    
    auto* node = new CallExpression();
    node->name = funcName;
    for (auto& a : args)
        node->arguments.emplace_back(std::move(a));
    
    return node;
}

Expression* parseArrayLiteral(Parser &p)
{
    advance(p);
    
    std::vector<std::unique_ptr<Expression>> elements;
    
    if (current(p).type != TokenType::RightBracket)
    {
        Expression* elem = parseExpression(p);
        elements.emplace_back(elem);
        
        while (current(p).type == TokenType::Comma)
        {
            advance(p);
            elem = parseExpression(p);
            elements.emplace_back(elem);
        }
    }
    
    consume(p, TokenType::RightBracket, "Expected ']' after array elements");
    
    auto* node = new ArrayLiteral();
    for (auto& e : elements)
        node->elements.emplace_back(std::move(e));
    
    return node;
}

Expression* parseIndexExpression(Parser &p, const std::string& varName)
{
    advance(p);
    
    Expression* index = parseExpression(p);
    
    consume(p, TokenType::RightBracket, "Expected ']' after index");
    
    auto* node = new IndexExpression();
    auto* id = new Identifier();
    id->name = varName;
    node->array.reset(id);
    node->index.reset(index);
    
    return node;
}
