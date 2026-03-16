#pragma once

#include <unordered_map>
#include <string>
#include <variant>
#include <stack>
#include <vector>
#include "ast.h"

enum class ValueType
{
    Number,
    String,
    Bool,
    Null,
    Array
};

struct Value
{
    ValueType type = ValueType::Null;
    std::variant<double, std::string, bool, std::vector<Value>> data;
};

// For function return values
struct ReturnValue
{
    Value value;
};

struct Interpreter
{
    std::unordered_map<std::string, Value> variables;
    std::unordered_map<std::string, std::string> varTypes;  // Track declared types
    std::unordered_map<std::string, FunctionDeclaration*> functions;  // Function table
    std::stack<std::unordered_map<std::string, Value>> scopeStack;  // Scope stack
};

Value evaluateExpression(Interpreter& interp, Expression* expr);

void executeStatement(Interpreter& interp, Statement* stmt);

void executeProgram(Interpreter& interp, Program& program);

// Utility functions
std::string valueToString(const Value& v);
void checkType(const std::string& declared, const Value& value);