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
Expression* parseArrayLiteral(Parser &p);
Expression* parseIndexExpression(Parser &p, const std::string& varName);

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

    // Detect function declaration: fn <type> <name>(...)
    if (t.type == TokenType::Function || (t.type == TokenType::Identifier && t.value == "fn"))
    {
        return parseFunctionDeclaration(p);
    }

    if (t.type == TokenType::TypeInt ||
        t.type == TokenType::TypeFloat ||
        t.type == TokenType::TypeString ||
        t.type == TokenType::TypeBool)
    {
        return parseVariableDeclaration(p);
    }

    // Detect return statement
    if (t.type == TokenType::Return || (t.type == TokenType::Identifier && t.value == "return"))
    {
        return parseReturnStatement(p);
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
        // Look ahead to see if next token is (
        size_t savedPos = p.position;
        Token next = current(p);
        advance(p);
        Token afterNext = current(p);
        p.position = savedPos;  // Reset position
        
        // Function call: identifier (
        if (afterNext.type == TokenType::LeftParen)
        {
            Expression* expr = parseExpression(p);
            consume(p, TokenType::Semicolon, "Expected ';' after expression");
            // Wrap in expression statement - for now just evaluate and discard
            auto* node = new AssignmentStatement();  // Temporary: wrap as assignment
            node->name = "__expr_stmt__";
            node->value.reset(expr);
            return node;
        }
        
        // Assignment: identifier =
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
    // Handle unary minus
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

        // Check if this is indexing: identifier [
        if (current(p).type == TokenType::LeftBracket)
        {
            return parseIndexExpression(p, t.value);
        }

        // Check if this is a function call: identifier (
        if (current(p).type == TokenType::LeftParen)
        {
            return parseCallExpression(p, t.value);
        }

        auto* node = new Identifier();
        node->name = t.value;
        return node;
    }

    // Array literal: [1, 2, 3]
    if (t.type == TokenType::LeftBracket)
    {
        return parseArrayLiteral(p);
    }

    error(t.line, t.column, "Unexpected token in expression");
    exit(1);
}

Statement* parseFunctionDeclaration(Parser &p)
{
    // Skip "fn" or "function" keyword
    advance(p);
    
    // Get return type
    Token returnType = consume(p, TokenType::TypeInt, "Expected return type");
    
    // Get function name
    Token name = consume(p, TokenType::Identifier, "Expected function name");
    
    // Expect opening parenthesis
    consume(p, TokenType::LeftParen, "Expected '(' after function name");
    
    // Parse parameters
    std::vector<std::string> paramTypes;
    std::vector<std::string> paramNames;
    
    if (current(p).type != TokenType::RightParen)
    {
        // First parameter
        Token ptype = consume(p, TokenType::TypeInt, "Expected parameter type");
        Token pname = consume(p, TokenType::Identifier, "Expected parameter name");
        paramTypes.push_back(ptype.value);
        paramNames.push_back(pname.value);
        
        // Additional parameters
        while (current(p).type == TokenType::Comma)
        {
            advance(p);  // skip comma
            ptype = consume(p, TokenType::TypeInt, "Expected parameter type");
            pname = consume(p, TokenType::Identifier, "Expected parameter name");
            paramTypes.push_back(ptype.value);
            paramNames.push_back(pname.value);
        }
    }
    
    consume(p, TokenType::RightParen, "Expected ')' after parameters");
    consume(p, TokenType::LeftBrace, "Expected '{' before function body");
    
    // Parse function body
    std::vector<std::unique_ptr<Statement>> body;
    while (current(p).type != TokenType::RightBrace)
    {
        Statement* stmt = parseStatement(p);
        if (stmt)
            body.emplace_back(stmt);
    }
    
    consume(p, TokenType::RightBrace, "Expected '}' after function body");
    
    auto* node = new FunctionDeclaration();
    node->returnType = returnType.value;
    node->name = name.value;
    node->paramTypes = paramTypes;
    node->paramNames = paramNames;
    for (auto& s : body)
        node->body.emplace_back(std::move(s));
    
    return node;
}

Statement* parseReturnStatement(Parser &p)
{
    // Skip "return" keyword
    advance(p);
    
    Expression* value = parseExpression(p);
    
    consume(p, TokenType::Semicolon, "Expected ';' after return value");
    
    auto* node = new ReturnStatement();
    node->value.reset(value);
    
    return node;
}

Expression* parseCallExpression(Parser &p, const std::string& funcName)
{
    // We've already consumed the function name and '('
    // Current token should be '('
    
    consume(p, TokenType::LeftParen, "Expected '(' in function call");
    
    // Parse arguments
    std::vector<std::unique_ptr<Expression>> args;
    
    if (current(p).type != TokenType::RightParen)
    {
        Expression* arg = parseExpression(p);
        args.emplace_back(arg);
        
        while (current(p).type == TokenType::Comma)
        {
            advance(p);  // skip comma
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
    advance(p);  // skip '['
    
    std::vector<std::unique_ptr<Expression>> elements;
    
    if (current(p).type != TokenType::RightBracket)
    {
        Expression* elem = parseExpression(p);
        elements.emplace_back(elem);
        
        while (current(p).type == TokenType::Comma)
        {
            advance(p);  // skip comma
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
    // We've consumed the identifier, current is '['
    advance(p);  // skip '['
    
    Expression* index = parseExpression(p);
    
    consume(p, TokenType::RightBracket, "Expected ']' after index");
    
    auto* node = new IndexExpression();
    auto* id = new Identifier();
    id->name = varName;
    node->array.reset(id);
    node->index.reset(index);
    
    return node;
}