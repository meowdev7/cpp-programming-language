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
        if (value.type != "number")
        {
            error(0,0,"Type error: expected number");
            exit(1);
        }
    }

    if (declared == "string")
    {
        if (value.type != "string")
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
        return {"number", num->value};
    }

    if (auto str = dynamic_cast<StringLiteral *>(expr))
    {
        return {"string", str->value};
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

        if (left.type != "number" || right.type != "number")
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
            return {"number", std::to_string(static_cast<long long>(result))};
        else
            return {"number", formatNumber(result)};
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