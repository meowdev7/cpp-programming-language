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

// Forward declarations for precedence levels
Expression* parseComparison(Parser &p);
Expression* parseTerm(Parser &p);
Expression* parseFactor(Parser &p);

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

    // Detect if statement
    if ((t.type == TokenType::Identifier && t.value == "if") || t.type == TokenType::If)
    {
        return parseIfStatement(p);
    }

    // Detect while statement
    if ((t.type == TokenType::Identifier && t.value == "while") || t.type == TokenType::While)
    {
        return parseWhileStatement(p);
    }

    // Detect assignment: identifier = expression
    if (t.type == TokenType::Identifier)
    {
        // Look ahead to see if next token is =
        size_t savedPos = p.position;
        Token next = current(p);
        advance(p);
        Token afterNext = current(p);
        p.position = savedPos;  // Reset position
        
        if (next.type == TokenType::Identifier && afterNext.type == TokenType::Equal)
        {
            return parseAssignment(p);
        }
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

Statement* parseIfStatement(Parser &p)
{
    advance(p);  // skip "if"
    
    consume(
        p,
        TokenType::LeftParen,
        "Expected '(' after 'if'"
    );
    
    Expression* condition = parseExpression(p);
    
    consume(
        p,
        TokenType::RightParen,
        "Expected ')' after condition"
    );
    
    consume(
        p,
        TokenType::LeftBrace,
        "Expected '{' after condition"
    );
    
    // Parse body statements until }
    std::vector<std::unique_ptr<Statement>> body;
    while (current(p).type != TokenType::RightBrace)
    {
        Statement* stmt = parseStatement(p);
        if (stmt)
            body.emplace_back(stmt);
    }
    
    consume(
        p,
        TokenType::RightBrace,
        "Expected '}' after if body"
    );
    
    auto* node = new IfStatement();
    node->condition.reset(condition);
    for (auto& s : body)
        node->body.emplace_back(std::move(s));
    
    return node;
}

Statement* parseAssignment(Parser &p)
{
    Token nameToken = current(p);
    advance(p);  // skip variable name
    advance(p);  // skip =
    
    Expression* value = parseExpression(p);
    
    consume(
        p,
        TokenType::Semicolon,
        "Expected ';' after assignment"
    );
    
    auto* node = new AssignmentStatement();
    node->name = nameToken.value;
    node->value.reset(value);
    
    return node;
}

Statement* parseWhileStatement(Parser &p)
{
    advance(p);  // skip "while"
    
    consume(
        p,
        TokenType::LeftParen,
        "Expected '(' after 'while'"
    );
    
    Expression* condition = parseExpression(p);
    
    consume(
        p,
        TokenType::RightParen,
        "Expected ')' after condition"
    );
    
    consume(
        p,
        TokenType::LeftBrace,
        "Expected '{' after condition"
    );
    
    // Parse body statements until }
    std::vector<std::unique_ptr<Statement>> body;
    while (current(p).type != TokenType::RightBrace)
    {
        Statement* stmt = parseStatement(p);
        if (stmt)
            body.emplace_back(stmt);
    }
    
    consume(
        p,
        TokenType::RightBrace,
        "Expected '}' after while body"
    );
    
    auto* node = new WhileStatement();
    node->condition.reset(condition);
    for (auto& s : body)
        node->body.emplace_back(std::move(s));
    
    return node;
}

Expression* parseExpression(Parser &p)
{
    return parseBinaryExpression(p);
}

// Parse comparison operators (lowest precedence)
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

// Parse addition/subtraction (higher than comparison)
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

// Parse multiplication/division (highest arithmetic precedence)
Expression* parseFactor(Parser &p)
{
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

Expression* parseBinaryExpression(Parser &p)
{
    return parseComparison(p);
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

        auto* node = new Identifier();
        node->name = t.value;
        return node;
    }

    error(t.line, t.column, "Unexpected token in expression");
    exit(1);
}