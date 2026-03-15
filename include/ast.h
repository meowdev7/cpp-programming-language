#pragma once

#include <string>
#include <vector>
#include <memory>

struct Node
{
    virtual ~Node() = default;
};

struct Expression : Node
{
};

struct Statement : Node
{
};

struct NumberLiteral : Expression
{
    std::string value;
};

struct StringLiteral : Expression
{
    std::string value;
};

struct BooleanLiteral : Expression
{
    bool value;
};

struct Identifier : Expression
{
    std::string name;
};

struct AssignmentExpression : Expression
{
    std::string name;
    std::unique_ptr<Expression> value;
};

struct AssignmentStatement : Statement
{
    std::string name;
    std::unique_ptr<Expression> value;
};

struct BinaryExpression : Expression
{
    std::unique_ptr<Expression> left;
    std::string op;
    std::unique_ptr<Expression> right;
};

struct VariableDeclaration : Statement
{
    std::string type;
    std::string name;
    std::unique_ptr<Expression> value;
};

struct PrintStatement : Statement
{
    std::unique_ptr<Expression> value;
};

struct IfStatement : Statement
{
    std::unique_ptr<Expression> condition;
    std::vector<std::unique_ptr<Statement>> body;
};

struct WhileStatement : Statement
{
    std::unique_ptr<Expression> condition;
    std::vector<std::unique_ptr<Statement>> body;
};

struct Program : Node
{
    std::vector<std::unique_ptr<Statement>> statements;
};