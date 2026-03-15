#pragma once

#include <unordered_map>
#include <string>
#include "ast.h"

enum class ValueType
{
    Number,
    String,
    Bool
};

struct Value
{
    ValueType type;
    std::string value;
};

struct Interpreter
{
    std::unordered_map<std::string, Value> variables;
};

Value evaluateExpression(Interpreter& interp, Expression* expr);

void executeStatement(Interpreter& interp, Statement* stmt);

void executeProgram(Interpreter& interp, Program& program);