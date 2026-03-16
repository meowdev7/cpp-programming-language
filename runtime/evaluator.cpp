#include "interpreter.h"
#include "error.h"
#include <iostream>

Value evaluateExpression(Interpreter &interp, Expression *expr)
{
    if (auto num = dynamic_cast<NumberLiteral *>(expr))
        return {ValueType::Number, std::stod(num->value)};

    if (auto str = dynamic_cast<StringLiteral *>(expr))
        return {ValueType::String, str->value};

    if (auto b = dynamic_cast<BooleanLiteral *>(expr))
        return {ValueType::Bool, b->value};

    if (auto id = dynamic_cast<Identifier *>(expr))
    {
        if (!interp.scopeStack.empty() && interp.scopeStack.top().count(id->name) > 0)
            return interp.scopeStack.top()[id->name];
        
        if (interp.variables.count(id->name) == 0)
        {
            error(0,0,"Undefined variable: " + id->name);
            exit(1);
        }
        return interp.variables[id->name];
    }

    if (auto call = dynamic_cast<CallExpression *>(expr))
    {
        if (interp.functions.count(call->name) == 0)
        {
            error(0,0,"Undefined function: " + call->name);
            exit(1);
        }
        
        FunctionDeclaration* func = interp.functions[call->name];
        
        std::vector<Value> argValues;
        for (auto& arg : call->arguments)
            argValues.push_back(evaluateExpression(interp, arg.get()));
        
        interp.scopeStack.push(std::unordered_map<std::string, Value>());
        
        for (size_t i = 0; i < func->paramNames.size() && i < argValues.size(); i++)
            interp.scopeStack.top()[func->paramNames[i]] = argValues[i];
        
        Value result;
        result.type = ValueType::Null;
        
        try
        {
            for (auto& stmt : func->body)
                executeStatement(interp, stmt.get());
        }
        catch (ReturnValue& ret)
        {
            result = ret.value;
        }
        
        interp.scopeStack.pop();
        return result;
    }

    if (auto bin = dynamic_cast<BinaryExpression *>(expr))
    {
        Value left = evaluateExpression(interp, bin->left.get());
        Value right = evaluateExpression(interp, bin->right.get());

        if (bin->op == "==" || bin->op == "!=" || bin->op == ">" || 
            bin->op == "<" || bin->op == ">=" || bin->op == "<=")
        {
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

    if (auto arr = dynamic_cast<ArrayLiteral *>(expr))
    {
        std::vector<Value> elements;
        for (auto& elem : arr->elements)
            elements.push_back(evaluateExpression(interp, elem.get()));
        return {ValueType::Array, elements};
    }

    if (auto idx = dynamic_cast<IndexExpression *>(expr))
    {
        Value arr = evaluateExpression(interp, idx->array.get());
        Value indexVal = evaluateExpression(interp, idx->index.get());
        
        if (arr.type != ValueType::Array)
        {
            error(0,0,"Cannot index non-array");
            exit(1);
        }
        
        if (indexVal.type != ValueType::Number)
        {
            error(0,0,"Array index must be a number");
            exit(1);
        }
        
        int index = static_cast<int>(std::get<double>(indexVal.data));
        auto& vec = std::get<std::vector<Value>>(arr.data);
        
        if (index < 0 || index >= static_cast<int>(vec.size()))
        {
            error(0,0,"Array index out of bounds");
            exit(1);
        }
        
        return vec[index];
    }

    error(0,0,"Unknown expression");
    exit(1);
}
