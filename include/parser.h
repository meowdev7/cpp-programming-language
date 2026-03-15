#pragma once

#include <vector>
#include "lexer.h"
#include "ast.h"

struct Parser
{
    std::vector<Token> tokens;
    size_t position = 0;
};

Token current(Parser &p);
Token advance(Parser &p);
bool match(Parser &p, TokenType type);

Program parseProgram(Parser &p);

Statement* parseStatement(Parser &p);
Statement* parseVariableDeclaration(Parser &p);
Statement* parsePrintStatement(Parser &p);
Statement* parseIfStatement(Parser &p);

Expression* parseExpression(Parser &p);
Expression* parseBinaryExpression(Parser &p);
Expression* parsePrimary(Parser &p);

bool isOperator(TokenType type);