#include "lexer.h"
#include "error.h"

std::string tokenTypeToString(TokenType type)
{
    switch (type)
    {
        case TokenType::Identifier: return "IDENTIFIER";
        case TokenType::Number: return "NUMBER";
        case TokenType::String: return "STRING";
        case TokenType::Let: return "LET";
        case TokenType::Const: return "CONST";
        case TokenType::Function: return "FUNCTION";
        case TokenType::Return: return "RETURN";
        case TokenType::If: return "IF";
        case TokenType::Else: return "ELSE";
        case TokenType::While: return "WHILE";
        case TokenType::For: return "FOR";
        case TokenType::TypeInt: return "TYPE_INT";
        case TokenType::TypeFloat: return "TYPE_FLOAT";
        case TokenType::TypeString: return "TYPE_STRING";
        case TokenType::TypeBool: return "TYPE_BOOL";
        case TokenType::True: return "TRUE";
        case TokenType::False: return "FALSE";
        case TokenType::Plus: return "PLUS";
        case TokenType::Minus: return "MINUS";
        case TokenType::Star: return "STAR";
        case TokenType::Slash: return "SLASH";
        case TokenType::Equal: return "EQUAL";
        case TokenType::EqualEqual: return "EQUAL_EQUAL";
        case TokenType::BangEqual: return "BANG_EQUAL";
        case TokenType::Greater: return "GREATER";
        case TokenType::Less: return "LESS";
        case TokenType::GreaterEqual: return "GREATER_EQUAL";
        case TokenType::LessEqual: return "LESS_EQUAL";
        case TokenType::LeftParen: return "LPAREN";
        case TokenType::RightParen: return "RPAREN";
        case TokenType::LeftBrace: return "LBRACE";
        case TokenType::RightBrace: return "RBRACE";
        case TokenType::LeftBracket: return "LBRACKET";
        case TokenType::RightBracket: return "RBRACKET";
        case TokenType::Semicolon: return "SEMICOLON";
        case TokenType::Comma: return "COMMA";
        case TokenType::EndOfFile: return "EOF";
        default: return "UNKNOWN";
    }
}
