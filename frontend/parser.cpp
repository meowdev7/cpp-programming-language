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
    if (current(p).type == type) {
        Token token = current(p);  // Capture current token before advancing
        advance(p);  // Then advance to next position
        return token;  // Return the captured token
    }

    error(current(p).line, current(p).column, msg);
    exit(1);
}

bool isOperator(TokenType type)
{
    return type == TokenType::Plus ||
           type == TokenType::Minus ||
           type == TokenType::Star ||
           type == TokenType::Slash ||
           type == TokenType::EqualEqual ||
           type == TokenType::BangEqual ||
           type == TokenType::Greater ||
           type == TokenType::Less ||
           type == TokenType::GreaterEqual ||
           type == TokenType::LessEqual;
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

    // Detect print statement
    if (t.type == TokenType::Identifier && t.value == "print")
    {
        return parsePrintStatement(p);
    }

    // FIX: Advance past unknown token to prevent infinite loop
    error(t.line, t.column, "Unknown statement");
    advance(p);
    return nullptr;
}

Statement* parseVariableDeclaration(Parser &p)
{
    Token typeToken = current(p);  // Get current token (the type), don't advance yet
    advance(p);  // Now advance to get the variable name

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

Statement* parsePrintStatement(Parser &p)
{
    advance(p);  // skip "print"
    
    consume(
        p,
        TokenType::LeftParen,
        "Expected '(' after 'print'"
    );
    
    Expression* value = parseExpression(p);
    
    consume(
        p,
        TokenType::RightParen,
        "Expected ')' after expression"
    );
    
    consume(
        p,
        TokenType::Semicolon,
        "Expected ';' after print statement"
    );
    
    auto* node = new PrintStatement();
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
        Token op = current(p);  // Get current operator before advancing
        advance(p);  // Advance to right operand

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
        // Check for boolean literals
        if (t.value == "true" || t.value == "false")
        {
            advance(p);
            auto* node = new BooleanLiteral();
            node->value = (t.value == "true");
            return node;
        }

        advance(p);

        auto* node = new Identifier();
        node->name = t.value;
        return node;
    }

    error(t.line, t.column, "Unexpected token in expression");
    exit(1);
}