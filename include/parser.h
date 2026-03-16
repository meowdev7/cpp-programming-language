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
Token consume(Parser &p, TokenType type, const std::string &msg);

Program parseProgram(Parser &p);

Statement* parseStatement(Parser &p);
Statement* parseVariableDeclaration(Parser &p);
Statement* parsePrintStatement(Parser &p);
Statement* parseIfStatement(Parser &p);
Statement* parseAssignment(Parser &p);
Statement* parseWhileStatement(Parser &p);
Statement* parseFunctionDeclaration(Parser &p);
Statement* parseReturnStatement(Parser &p);

Expression* parseExpression(Parser &p);
Expression* parseBinaryExpression(Parser &p);
Expression* parseComparison(Parser &p);
Expression* parseTerm(Parser &p);
Expression* parseFactor(Parser &p);
Expression* parsePrimary(Parser &p);
Expression* parseCallExpression(Parser &p, const std::string &funcName);
Expression* parseArrayLiteral(Parser &p);
Expression* parseIndexExpression(Parser &p, const std::string &varName);

bool isOperator(TokenType type);