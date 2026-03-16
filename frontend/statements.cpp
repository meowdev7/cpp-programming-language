#include "parser.h"
#include "error.h"
#include <iostream>

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

    if (t.type == TokenType::Function || (t.type == TokenType::Identifier && t.value == "fn"))
        return parseFunctionDeclaration(p);

    if (t.type == TokenType::TypeInt || t.type == TokenType::TypeFloat ||
        t.type == TokenType::TypeString || t.type == TokenType::TypeBool)
        return parseVariableDeclaration(p);

    if (t.type == TokenType::Return || (t.type == TokenType::Identifier && t.value == "return"))
        return parseReturnStatement(p);

    if (t.type == TokenType::Identifier && t.value == "print")
        return parsePrintStatement(p);

    if ((t.type == TokenType::Identifier && t.value == "if") || t.type == TokenType::If)
        return parseIfStatement(p);

    if ((t.type == TokenType::Identifier && t.value == "while") || t.type == TokenType::While)
        return parseWhileStatement(p);

    if (t.type == TokenType::Identifier)
    {
        size_t savedPos = p.position;
        Token next = current(p);
        advance(p);
        Token afterNext = current(p);
        p.position = savedPos;
        
        if (afterNext.type == TokenType::LeftParen)
        {
            Expression* expr = parseExpression(p);
            consume(p, TokenType::Semicolon, "Expected ';' after expression");
            auto* node = new AssignmentStatement();
            node->name = "__expr_stmt__";
            node->value.reset(expr);
            return node;
        }
        
        if (next.type == TokenType::Identifier && afterNext.type == TokenType::Equal)
            return parseAssignment(p);
    }

    error(t.line, t.column, "Unknown statement");
    advance(p);
    return nullptr;
}

Statement* parseVariableDeclaration(Parser &p)
{
    Token typeToken = current(p);
    advance(p);

    Token nameToken = consume(p, TokenType::Identifier, "Expected variable name after type");
    consume(p, TokenType::Equal, "Expected '=' after variable name");

    Expression* value = parseExpression(p);
    consume(p, TokenType::Semicolon, "Expected ';' after declaration");

    auto* node = new VariableDeclaration();
    node->type = typeToken.value;
    node->name = nameToken.value;
    node->value.reset(value);
    return node;
}

Statement* parsePrintStatement(Parser &p)
{
    advance(p);
    consume(p, TokenType::LeftParen, "Expected '(' after 'print'");
    
    Expression* value = parseExpression(p);
    consume(p, TokenType::RightParen, "Expected ')' after expression");
    consume(p, TokenType::Semicolon, "Expected ';' after print statement");
    
    auto* node = new PrintStatement();
    node->value.reset(value);
    return node;
}

Statement* parseIfStatement(Parser &p)
{
    advance(p);
    consume(p, TokenType::LeftParen, "Expected '(' after 'if'");
    
    Expression* condition = parseExpression(p);
    consume(p, TokenType::RightParen, "Expected ')' after condition");
    consume(p, TokenType::LeftBrace, "Expected '{' after condition");
    
    std::vector<std::unique_ptr<Statement>> body;
    while (current(p).type != TokenType::RightBrace)
    {
        Statement* stmt = parseStatement(p);
        if (stmt)
            body.emplace_back(stmt);
    }
    
    consume(p, TokenType::RightBrace, "Expected '}' after if body");
    
    auto* node = new IfStatement();
    node->condition.reset(condition);
    for (auto& s : body)
        node->body.emplace_back(std::move(s));
    return node;
}

Statement* parseAssignment(Parser &p)
{
    Token nameToken = current(p);
    advance(p);
    advance(p);
    
    Expression* value = parseExpression(p);
    consume(p, TokenType::Semicolon, "Expected ';' after assignment");
    
    auto* node = new AssignmentStatement();
    node->name = nameToken.value;
    node->value.reset(value);
    return node;
}

Statement* parseWhileStatement(Parser &p)
{
    advance(p);
    consume(p, TokenType::LeftParen, "Expected '(' after 'while'");
    
    Expression* condition = parseExpression(p);
    consume(p, TokenType::RightParen, "Expected ')' after condition");
    consume(p, TokenType::LeftBrace, "Expected '{' after condition");
    
    std::vector<std::unique_ptr<Statement>> body;
    while (current(p).type != TokenType::RightBrace)
    {
        Statement* stmt = parseStatement(p);
        if (stmt)
            body.emplace_back(stmt);
    }
    
    consume(p, TokenType::RightBrace, "Expected '}' after while body");
    
    auto* node = new WhileStatement();
    node->condition.reset(condition);
    for (auto& s : body)
        node->body.emplace_back(std::move(s));
    return node;
}

Statement* parseFunctionDeclaration(Parser &p)
{
    advance(p);
    
    Token returnType = consume(p, TokenType::TypeInt, "Expected return type");
    Token name = consume(p, TokenType::Identifier, "Expected function name");
    consume(p, TokenType::LeftParen, "Expected '(' after function name");
    
    std::vector<std::string> paramTypes;
    std::vector<std::string> paramNames;
    
    if (current(p).type != TokenType::RightParen)
    {
        Token ptype = consume(p, TokenType::TypeInt, "Expected parameter type");
        Token pname = consume(p, TokenType::Identifier, "Expected parameter name");
        paramTypes.push_back(ptype.value);
        paramNames.push_back(pname.value);
        
        while (current(p).type == TokenType::Comma)
        {
            advance(p);
            ptype = consume(p, TokenType::TypeInt, "Expected parameter type");
            pname = consume(p, TokenType::Identifier, "Expected parameter name");
            paramTypes.push_back(ptype.value);
            paramNames.push_back(pname.value);
        }
    }
    
    consume(p, TokenType::RightParen, "Expected ')' after parameters");
    consume(p, TokenType::LeftBrace, "Expected '{' before function body");
    
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
    advance(p);
    
    Expression* value = parseExpression(p);
    consume(p, TokenType::Semicolon, "Expected ';' after return value");
    
    auto* node = new ReturnStatement();
    node->value.reset(value);
    return node;
}
