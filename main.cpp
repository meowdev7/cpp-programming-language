#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include "lexer.h"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: zor <file.zr>\n";
        return 1;
    }

    std::ifstream file(argv[1]);

    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file " << argv[1] << "\n";
        return 1;
    }

    std::string source(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

    Lexer lexer{source, 0};

    std::vector<Token> tokens = lex(lexer);

    for (auto &token : tokens)
    {
        std::cout << "Token: " << token.value
                  << " (Type: " << tokenTypeToString(token.type) << ")\n";
    }

    return 0;
}