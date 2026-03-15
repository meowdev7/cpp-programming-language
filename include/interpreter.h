#pragma once

#include <unordered_map>
#include <string>
#include <variant>
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
    std::variant<double, std::string, bool> data;
};

struct Interpreter
{
    std::unordered_map<std::string, Value> variables;
    std::unordered_map<std::string, std::string> varTypes;  // Track declared types
};

Value evaluateExpression(Interpreter& interp, Expression* expr);

void executeStatement(Interpreter& interp, Statement* stmt);

void executeProgram(Interpreter& interp, Program& program);