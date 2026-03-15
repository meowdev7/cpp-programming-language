#pragma once

#include <unordered_map>
#include <string>
#include "ast.h"

struct Value
{
    std::string type;
    std::string value;
};

struct Interpreter
{
    std::unordered_map<std::string, Value> variables;
};

Value evaluateExpression(Interpreter& interp, Expression* expr);

void executeStatement(Interpreter& interp, Statement* stmt);

void executeProgram(Interpreter& interp, Program& program);