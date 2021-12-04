#pragma once

#include <string_view> // this is STL, but it does not need exceptions


class CFnMatch
{
public:
    static bool Match(const std::string_view text, const std::string_view pattern);
};
