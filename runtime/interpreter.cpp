#include "interpreter.h"
#include "error.h"

#include <iostream>
#include <iomanip>
#include <variant>

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

std::string valueToString(const Value& v)
{
    if (v.type == ValueType::Number)
    {
        double num = std::get<double>(v.data);
        if (num == static_cast<long long>(num))
            return std::to_string(static_cast<long long>(num));
        return formatNumber(num);
    }
    else if (v.type == ValueType::String)
    {
        return std::get<std::string>(v.data);
    }
    else if (v.type == ValueType::Bool)
    {
        return std::get<bool>(v.data) ? "true" : "false";
    }
    return "";
}

Value evaluateExpression(Interpreter &interp, Expression *expr)
{
    if (auto num = dynamic_cast<NumberLiteral *>(expr))
    {
        return {ValueType::Number, std::stod(num->value)};
    }

    if (auto str = dynamic_cast<StringLiteral *>(expr))
    {
        return {ValueType::String, str->value};
    }

    if (auto b = dynamic_cast<BooleanLiteral *>(expr))
    {
        return {ValueType::Bool, b->value};
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
                double a = std::get<double>(left.data);
                double b = std::get<double>(right.data);

                if (bin->op == "==") result = (a == b);
                else if (bin->op == "!=") result = (a != b);
                else if (bin->op == ">") result = (a > b);
                else if (bin->op == "<") result = (a < b);
                else if (bin->op == ">=") result = (a >= b);
                else if (bin->op == "<=") result = (a <= b);
            }
            else if (left.type == ValueType::String)
            {
                const std::string& a = std::get<std::string>(left.data);
                const std::string& b = std::get<std::string>(right.data);
                if (bin->op == "==") result = (a == b);
                else if (bin->op == "!=") result = (a != b);
                else { error(0,0,"Cannot use comparison on strings"); exit(1); }
            }

            return {ValueType::Bool, result};
        }

        // Arithmetic operators
        if (left.type != ValueType::Number || right.type != ValueType::Number)
        {
            error(0,0,"Arithmetic requires numbers");
            exit(1);
        }

        double a = std::get<double>(left.data);
        double b = std::get<double>(right.data);
        double result;

        if (bin->op == "+") result = a + b;
        else if (bin->op == "-") result = a - b;
        else if (bin->op == "*") result = a * b;
        else if (bin->op == "/") result = a / b;
        else { error(0,0,"Unknown operator"); exit(1); }

        return {ValueType::Number, result};
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
        interp.varTypes[var->name] = var->type;  // Store declared type

        return;
    }

    if (auto print = dynamic_cast<PrintStatement *>(stmt))
    {
        Value value = evaluateExpression(interp, print->value.get());
        std::cout << valueToString(value) << std::endl;
        return;
    }

    if (auto ifStmt = dynamic_cast<IfStatement *>(stmt))
    {
        Value cond = evaluateExpression(interp, ifStmt->condition.get());
        
        if (cond.type != ValueType::Bool)
        {
            error(0,0,"Condition must be boolean");
            exit(1);
        }
        
        if (std::get<bool>(cond.data))
        {
            for (auto& stmt : ifStmt->body)
            {
                executeStatement(interp, stmt.get());
            }
        }
        return;
    }

    if (auto assign = dynamic_cast<AssignmentStatement *>(stmt))
    {
        Value value = evaluateExpression(interp, assign->value.get());
        
        if (interp.variables.count(assign->name) == 0)
        {
            error(0,0,"Undefined variable: " + assign->name);
            exit(1);
        }
        
        interp.variables[assign->name] = value;
        return;
    }

    if (auto whileStmt = dynamic_cast<WhileStatement *>(stmt))
    {
        while (true)
        {
            Value cond = evaluateExpression(interp, whileStmt->condition.get());
            
            if (cond.type != ValueType::Bool)
            {
                error(0,0,"Condition must be boolean");
                exit(1);
            }
            
            if (!std::get<bool>(cond.data))
                break;
            
            for (auto& stmt : whileStmt->body)
            {
                executeStatement(interp, stmt.get());
            }
        }
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
        std::cout << v.first << " = " << valueToString(v.second) << std::endl;
    }
}