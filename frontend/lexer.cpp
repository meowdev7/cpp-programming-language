#include <iostream>
#include <string>
#include <vector>
#include <cctype>

#include "lexer.h"

std::string tokenTypeToString(TokenType type)
{
    switch (type)
    {
    case TokenType::Identifier:
        return "IDENTIFIER";
    case TokenType::Number:
        return "NUMBER";
    case TokenType::String:
        return "STRING";
    case TokenType::Let:
        return "LET";
    case TokenType::Const:
        return "CONST";
    case TokenType::Function:
        return "FUNCTION";
    case TokenType::Return:
        return "RETURN";
    case TokenType::If:
        return "IF";
    case TokenType::Else:
        return "ELSE";
    case TokenType::While:
        return "WHILE";
    case TokenType::For:
        return "FOR";
    case TokenType::Plus:
        return "PLUS";
    case TokenType::Minus:
        return "MINUS";
    case TokenType::Star:
        return "STAR";
    case TokenType::Slash:
        return "SLASH";
    case TokenType::Equal:
        return "EQUAL";
    case TokenType::LeftParen:
        return "LPAREN";
    case TokenType::RightParen:
        return "RPAREN";
    case TokenType::LeftBrace:
        return "LBRACE";
    case TokenType::RightBrace:
        return "RBRACE";
    case TokenType::Semicolon:
        return "SEMICOLON";
    case TokenType::Comma:
        return "COMMA";
    case TokenType::EndOfFile:
        return "EOF";
    default:
        return "UNKNOWN";
    }
}

char currentChar(Lexer &lexer)
{
    if (lexer.position >= lexer.source.size())
        return '\0';

    return lexer.source[lexer.position];
}

char advance(Lexer &lexer)
{
    lexer.position++;
    return currentChar(lexer);
}

bool isAlpha(char c)
{
    return std::isalpha(c) || c == '_';
}

bool isDigit(char c)
{
    return std::isdigit(c);
}

bool isAlphaNumeric(char c)
{
    return isAlpha(c) || isDigit(c);
}

Token makeNumber(Lexer &lexer)
{
    std::string value;

    while (isDigit(currentChar(lexer)))
    {
        value += currentChar(lexer);
        advance(lexer);
    }

    return {TokenType::Number, value};
}

Token makeIdentifier(Lexer &lexer)
{
    std::string value;

    while (isAlphaNumeric(currentChar(lexer)))
    {
        value += currentChar(lexer);
        advance(lexer);
    }

    if (value == "let")
        return {TokenType::Let, value};
    if (value == "const")
        return {TokenType::Const, value};
    if (value == "function")
        return {TokenType::Function, value};
    if (value == "return")
        return {TokenType::Return, value};
    if (value == "if")
        return {TokenType::If, value};
    if (value == "else")
        return {TokenType::Else, value};
    if (value == "while")
        return {TokenType::While, value};
    if (value == "for")
        return {TokenType::For, value};

    return {TokenType::Identifier, value};
}

std::vector<Token> lex(Lexer &lexer)
{
    std::vector<Token> tokens;

    while (true)
    {
        char c = currentChar(lexer);

        if (std::isspace(c))
        {
            advance(lexer);
            continue;
        }

        if (c == '\0')
        {
            tokens.push_back({TokenType::EndOfFile, ""});
            break;
        }

        if (isDigit(c))
        {
            tokens.push_back(makeNumber(lexer));
            continue;
        }

        if (isAlpha(c))
        {
            tokens.push_back(makeIdentifier(lexer));
            continue;
        }

        switch (c)
        {
        case '+':
            tokens.push_back({TokenType::Plus, "+"});
            break;

        case '-':
            tokens.push_back({TokenType::Minus, "-"});
            break;

        case '*':
            tokens.push_back({TokenType::Star, "*"});
            break;

        case '/':
            tokens.push_back({TokenType::Slash, "/"});
            break;

        case '=':
            tokens.push_back({TokenType::Equal, "="});
            break;

        case '(':
            tokens.push_back({TokenType::LeftParen, "("});
            break;

        case ')':
            tokens.push_back({TokenType::RightParen, ")"});
            break;

        case '{':
            tokens.push_back({TokenType::LeftBrace, "{"});
            break;

        case '}':
            tokens.push_back({TokenType::RightBrace, "}"});
            break;

        case ';':
            tokens.push_back({TokenType::Semicolon, ";"});
            break;

        case ',':
            tokens.push_back({TokenType::Comma, ","});
            break;

        default:
            std::cerr << "Unknown character: " << c << "\n";
        }

        advance(lexer);
    }

    return tokens;
}
