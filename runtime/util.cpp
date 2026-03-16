#include "interpreter.h"
#include "error.h"
#include <iostream>
#include <iomanip>

std::string formatNumber(double value)
{
    std::string s = std::to_string(value);
    while (!s.empty() && s.back() == '0' && s.find('.') != std::string::npos)
        s.pop_back();
    if (!s.empty() && s.back() == '.')
        s.push_back('0');
    return s;
}

void checkType(const std::string& declared, const Value& value)
{
    if (declared == "int" || declared == "float")
    {
        if (value.type != ValueType::Number && value.type != ValueType::Array)
        {
            error(0,0,"Type error: expected number");
            exit(1);
        }
    }

    if (declared == "string")
    {
        if (value.type != ValueType::String)
        {
            error(0,0,"Type error: expected string");
            exit(1);
        }
    }

    if (declared == "bool")
    {
        if (value.type != ValueType::Bool)
        {
            error(0,0,"Type error: expected bool");
            exit(1);
        }
    }
}

std::string valueToString(const Value& v)
{
    if (v.type == ValueType::Number)
    {
        double num = std::get<double>(v.data);
        if (num == static_cast<long long>(num))
            return std::to_string(static_cast<long long>(num));
        return formatNumber(num);
    }
    else if (v.type == ValueType::String)
        return std::get<std::string>(v.data);
    else if (v.type == ValueType::Bool)
        return std::get<bool>(v.data) ? "true" : "false";
    else if (v.type == ValueType::Array)
    {
        auto& arr = std::get<std::vector<Value>>(v.data);
        std::string result = "[";
        for (size_t i = 0; i < arr.size(); i++)
        {
            if (i > 0) result += ", ";
            result += valueToString(arr[i]);
        }
        result += "]";
        return result;
    }
    return "";
}
