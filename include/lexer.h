#pragma once
#include <string>
#include <vector>

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

    TypeInt,
    TypeFloat,
    TypeString,
    TypeBool,

    True,
    False,

    Plus,
    Minus,
    Star,
    Slash,

    Equal,
    EqualEqual,
    BangEqual,

    Greater,
    Less,
    GreaterEqual,
    LessEqual,

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

    int line;
    int column;
};

struct Lexer
{
    std::string source;
    size_t position = 0;

    int line = 1;
    int column = 1;
};

std::vector<Token> lex(Lexer &lexer);
std::string tokenTypeToString(TokenType type);
