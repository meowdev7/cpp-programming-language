#pragma once
#include <vector>
#include <string>

enum class TokenType
{
    Identifier,
    Number,
    String,
    Let,
    Const,
    Function,
    Return,
    If,
    Else,
    While,
    For,
    Plus,
    Minus,
    Star,
    Slash,
    Equal,
    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,
    Semicolon,
    Comma,
    EndOfFile
};

struct Token
{
    TokenType type;
    std::string value;
};

struct Lexer
{
    std::string source;
    int position;
};

std::vector<Token> lex(Lexer &lexer);
std::string tokenTypeToString(TokenType type);