#include "interpreter.h"
#include "error.h"
#include <iostream>

void executeStatement(Interpreter &interp, Statement *stmt)
{
    if (auto var = dynamic_cast<VariableDeclaration *>(stmt))
    {
        Value value = evaluateExpression(interp, var->value.get());
        checkType(var->type, value);
        interp.variables[var->name] = value;
        interp.varTypes[var->name] = var->type;
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
                executeStatement(interp, stmt.get());
        }
        return;
    }

    if (auto assign = dynamic_cast<AssignmentStatement *>(stmt))
    {
        if (assign->name == "__expr_stmt__")
        {
            evaluateExpression(interp, assign->value.get());
            return;
        }
        
        Value value = evaluateExpression(interp, assign->value.get());
        
        bool found = false;
        if (!interp.scopeStack.empty() && interp.scopeStack.top().count(assign->name) > 0)
        {
            interp.scopeStack.top()[assign->name] = value;
            found = true;
        }
        else if (interp.variables.count(assign->name) > 0)
        {
            interp.variables[assign->name] = value;
            found = true;
        }
        
        if (!found)
        {
            error(0,0,"Undefined variable: " + assign->name);
            exit(1);
        }
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
                executeStatement(interp, stmt.get());
        }
        return;
    }

    if (auto funcDecl = dynamic_cast<FunctionDeclaration *>(stmt))
    {
        interp.functions[funcDecl->name] = funcDecl;
        return;
    }

    if (auto ret = dynamic_cast<ReturnStatement *>(stmt))
    {
        Value value = evaluateExpression(interp, ret->value.get());
        throw ReturnValue{value};
    }

    if (auto assign = dynamic_cast<AssignmentStatement *>(stmt))
    {
        if (assign->name == "__expr_stmt__")
        {
            evaluateExpression(interp, assign->value.get());
            return;
        }
    }

    error(0,0,"Unknown statement");
    exit(1);
}

void executeProgram(Interpreter &interp, Program &program)
{
    for (auto &stmt : program.statements)
        executeStatement(interp, stmt.get());

    std::cout << "\n--- Variables ---\n";

    for (auto &v : interp.variables)
        std::cout << v.first << " = " << valueToString(v.second) << std::endl;
}
