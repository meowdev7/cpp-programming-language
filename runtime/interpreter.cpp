#include "interpreter.h"
#include "error.h"

#include <iostream>
#include <iomanip>

std::string formatNumber(double value)
{
    std::string s = std::to_string(value);
    // Remove trailing zeros but keep at least one decimal place if it's a float
    while (!s.empty() && s.back() == '0' && s.find('.') != std::string::npos) {
        s.pop_back();
    }
    // If we removed the last digit after decimal, keep one zero
    if (!s.empty() && s.back() == '.') {
        s.push_back('0');
    }
    return s;
}

void checkType(const std::string& declared, const Value& value)
{
    if (declared == "int" || declared == "float")
    {
        if (value.type != ValueType::Number)
        {
            error(0,0,"Type error: expected number");
            exit(1);
        }
    }

    if (declared == "string")
    {
        if (value.type != ValueType::String)
        {
            error(0,0,"Type error: expected string");
            exit(1);
        }
    }
}

Value evaluateExpression(Interpreter &interp, Expression *expr)
{
    if (auto num = dynamic_cast<NumberLiteral *>(expr))
    {
        return {ValueType::Number, num->value};
    }

    if (auto str = dynamic_cast<StringLiteral *>(expr))
    {
        return {ValueType::String, str->value};
    }

    if (auto b = dynamic_cast<BooleanLiteral *>(expr))
    {
        return {ValueType::Bool, b->value ? "true" : "false"};
    }

    if (auto id = dynamic_cast<Identifier *>(expr))
    {
        if (interp.variables.count(id->name) == 0)
        {
            error(0,0,"Undefined variable: " + id->name);
            exit(1);
        }

        return interp.variables[id->name];
    }

    if (auto bin = dynamic_cast<BinaryExpression *>(expr))
    {
        Value left = evaluateExpression(interp, bin->left.get());
        Value right = evaluateExpression(interp, bin->right.get());

        // Comparison operators
        if (bin->op == "==" || bin->op == "!=" || bin->op == ">" || 
            bin->op == "<" || bin->op == ">=" || bin->op == "<=")
        {
            // For comparisons, both must be same type
            if (left.type != right.type)
            {
                error(0,0,"Cannot compare different types");
                exit(1);
            }

            bool result = false;

            if (left.type == ValueType::Number)
            {
                double a = std::stod(left.value);
                double b = std::stod(right.value);

                if (bin->op == "==") result = (a == b);
                else if (bin->op == "!=") result = (a != b);
                else if (bin->op == ">") result = (a > b);
                else if (bin->op == "<") result = (a < b);
                else if (bin->op == ">=") result = (a >= b);
                else if (bin->op == "<=") result = (a <= b);
            }
            else if (left.type == ValueType::String)
            {
                if (bin->op == "==") result = (left.value == right.value);
                else if (bin->op == "!=") result = (left.value != right.value);
                else { error(0,0,"Cannot use comparison on strings"); exit(1); }
            }

            return {ValueType::Bool, result ? "true" : "false"};
        }

        // Arithmetic operators
        if (left.type != ValueType::Number || right.type != ValueType::Number)
        {
            error(0,0,"Arithmetic requires numbers");
            exit(1);
        }

        double a = std::stod(left.value);
        double b = std::stod(right.value);
        double result;

        if (bin->op == "+") result = a + b;
        else if (bin->op == "-") result = a - b;
        else if (bin->op == "*") result = a * b;
        else if (bin->op == "/") result = a / b;
        else { error(0,0,"Unknown operator"); exit(1); }

        // Format: show as integer if whole number, otherwise as float
        if (result == static_cast<long long>(result))
            return {ValueType::Number, std::to_string(static_cast<long long>(result))};
        else
            return {ValueType::Number, formatNumber(result)};
    }

    error(0,0,"Unknown expression");
    exit(1);
}

void executeStatement(Interpreter &interp, Statement *stmt)
{
    if (auto var = dynamic_cast<VariableDeclaration *>(stmt))
    {
        Value value = evaluateExpression(interp, var->value.get());

        checkType(var->type, value);

        interp.variables[var->name] = value;

        return;
    }

    if (auto print = dynamic_cast<PrintStatement *>(stmt))
    {
        Value value = evaluateExpression(interp, print->value.get());
        std::cout << value.value << std::endl;
        return;
    }

    error(0,0,"Unknown statement");
    exit(1);
}

void executeProgram(Interpreter &interp, Program &program)
{
    for (auto &stmt : program.statements)
    {
        executeStatement(interp, stmt.get());
    }

    // debug output
    std::cout << "\n--- Variables ---\n";

    for (auto &v : interp.variables)
    {
        std::cout << v.first << " = " << v.second.value << std::endl;
    }
}