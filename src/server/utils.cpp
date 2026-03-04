#include <vector>
#include <sstream>
#include "utils.h"

bool isNumber(const std::string& s)
{
    if (s.empty())
        return false;

    for (size_t i = 0; i < s.length(); ++i)
        if (!std::isdigit(s[i]))
            return false;
    return true;
}