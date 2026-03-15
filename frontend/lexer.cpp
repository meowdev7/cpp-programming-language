#include <iostream>
#include <string>
#include <vector>
#include <cctype>

#include "lexer.h"
#include "error.h"

char currentChar(Lexer &lexer)
{
    if (lexer.position >= lexer.source.size())
        return '\0';

    return lexer.source[lexer.position];
}

char peek(Lexer &lexer)
{
    size_t next = lexer.position + 1;

    if (next >= lexer.source.size())
        return '\0';

    return lexer.source[next];
}

char advance(Lexer &lexer)
{
    char c = currentChar(lexer);

    if (lexer.position < lexer.source.size())
        lexer.position++;

    if (c == '\n')
    {
        lexer.line++;
        lexer.column = 1;
    }
    else
    {
        lexer.column++;
    }

    return currentChar(lexer);
}

Token makeToken(Lexer &lexer, TokenType type, const std::string &value)
{
    return {type, value, lexer.line, lexer.column};
}

bool isAlpha(char c)
{
    return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}

bool isDigit(char c)
{
    return std::isdigit(static_cast<unsigned char>(c));
}

bool isAlphaNumeric(char c)
{
    return isAlpha(c) || isDigit(c);
}

Token makeNumber(Lexer &lexer)
{
    std::string value;
    bool hasDot = false;

    while (true)
    {
        char c = currentChar(lexer);

        if (isDigit(c))
        {
            value += c;
            advance(lexer);
            continue;
        }

        if (c == '.' && !hasDot && isDigit(peek(lexer)))
        {
            hasDot = true;
            value += c;
            advance(lexer);
            continue;
        }

        break;
    }

    return makeToken(lexer, TokenType::Number, value);
}

Token makeIdentifier(Lexer &lexer)
{
    std::string value;

    while (isAlphaNumeric(currentChar(lexer)))
    {
        value += currentChar(lexer);
        advance(lexer);
    }

    if (value == "let") return makeToken(lexer, TokenType::Let, value);
    if (value == "const") return makeToken(lexer, TokenType::Const, value);
    if (value == "function") return makeToken(lexer, TokenType::Function, value);
    if (value == "return") return makeToken(lexer, TokenType::Return, value);
    if (value == "if") return makeToken(lexer, TokenType::If, value);
    if (value == "else") return makeToken(lexer, TokenType::Else, value);
    if (value == "while") return makeToken(lexer, TokenType::While, value);
    if (value == "for") return makeToken(lexer, TokenType::For, value);

    if (value == "int") return makeToken(lexer, TokenType::TypeInt, value);
    if (value == "float") return makeToken(lexer, TokenType::TypeFloat, value);
    if (value == "string") return makeToken(lexer, TokenType::TypeString, value);
    if (value == "bool") return makeToken(lexer, TokenType::TypeBool, value);

    return makeToken(lexer, TokenType::Identifier, value);
}

Token makeString(Lexer &lexer)
{
    advance(lexer); // skip opening quote

    std::string value;

    while (true)
    {
        char c = currentChar(lexer);

        if (c == '\0')
        {
            error(lexer.line, lexer.column, "Unterminated string literal");
            break;
        }

        if (c == '"')
            break;

        value += c;
        advance(lexer);
    }

    advance(lexer); // skip closing quote

    return makeToken(lexer, TokenType::String, value);
}

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

        case TokenType::Semicolon: return "SEMICOLON";
        case TokenType::Comma: return "COMMA";

        case TokenType::EndOfFile: return "EOF";
        default: return "UNKNOWN";
    }
}

std::vector<Token> lex(Lexer &lexer)
{
    std::vector<Token> tokens;

    while (true)
    {
        char c = currentChar(lexer);

        if (std::isspace(static_cast<unsigned char>(c)))
        {
            advance(lexer);
            continue;
        }

        if (c == '\0')
        {
            tokens.push_back(makeToken(lexer, TokenType::EndOfFile, ""));
            break;
        }

        // single line comment
        if (c == '/' && peek(lexer) == '/')
        {
            while (currentChar(lexer) != '\n' && currentChar(lexer) != '\0')
                advance(lexer);

            continue;
        }

        // multi-line comment
        if (c == '/' && peek(lexer) == '*')
        {
            advance(lexer);
            advance(lexer);

            while (true)
            {
                if (currentChar(lexer) == '\0')
                {
                    error(lexer.line, lexer.column, "Unterminated comment");
                    return tokens;
                }

                if (currentChar(lexer) == '*' && peek(lexer) == '/')
                {
                    advance(lexer);
                    advance(lexer);
                    break;
                }

                advance(lexer);
            }

            continue;
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

        if (c == '"')
        {
            tokens.push_back(makeString(lexer));
            continue;
        }

        switch (c)
        {
            case '+': tokens.push_back(makeToken(lexer, TokenType::Plus, "+")); break;
            case '-': tokens.push_back(makeToken(lexer, TokenType::Minus, "-")); break;
            case '*': tokens.push_back(makeToken(lexer, TokenType::Star, "*")); break;
            case '/': tokens.push_back(makeToken(lexer, TokenType::Slash, "/")); break;

            case '=':
                if (peek(lexer) == '=')
                {
                    advance(lexer);
                    tokens.push_back(makeToken(lexer, TokenType::EqualEqual, "=="));
                }
                else
                    tokens.push_back(makeToken(lexer, TokenType::Equal, "="));
                break;

            case '!':
                if (peek(lexer) == '=')
                {
                    advance(lexer);
                    tokens.push_back(makeToken(lexer, TokenType::BangEqual, "!="));
                }
                else
                    error(lexer.line, lexer.column, "Unexpected '!'");
                break;

            case '>':
                if (peek(lexer) == '=')
                {
                    advance(lexer);
                    tokens.push_back(makeToken(lexer, TokenType::GreaterEqual, ">="));
                }
                else
                    tokens.push_back(makeToken(lexer, TokenType::Greater, ">"));
                break;

            case '<':
                if (peek(lexer) == '=')
                {
                    advance(lexer);
                    tokens.push_back(makeToken(lexer, TokenType::LessEqual, "<="));
                }
                else
                    tokens.push_back(makeToken(lexer, TokenType::Less, "<"));
                break;

            case '(':
                tokens.push_back(makeToken(lexer, TokenType::LeftParen, "("));
                break;

            case ')':
                tokens.push_back(makeToken(lexer, TokenType::RightParen, ")"));
                break;

            case '{':
                tokens.push_back(makeToken(lexer, TokenType::LeftBrace, "{"));
                break;

            case '}':
                tokens.push_back(makeToken(lexer, TokenType::RightBrace, "}"));
                break;

            case ';':
                tokens.push_back(makeToken(lexer, TokenType::Semicolon, ";"));
                break;

            case ',':
                tokens.push_back(makeToken(lexer, TokenType::Comma, ","));
                break;

            default:
                error(lexer.line, lexer.column, std::string("Unknown character: ") + c);
                break;
        }

        advance(lexer);
    }

    return tokens;
}