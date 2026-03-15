#include <iostream>
#include "error.h"

void error(int line, int column, const std::string &message)
{
    std::cerr << "[Error] "
              << line << ":" << column
              << " -> " << message
              << std::endl;
}