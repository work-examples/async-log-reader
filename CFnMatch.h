#pragma once

#include "CCharBuffer.h"

#include <string_view> // this is STL, but it does not need exceptions


class CFnMatch
{
public:
    bool SetPattern(const char* const pattern);  // set match filter; return false on error

    constexpr std::string_view GetPattern() const
    {
        return { this->_pattern.ptr, this->_pattern.size };
    }

    bool FullMatch(std::string_view text);

protected:
    CCharBuffer      _pattern;
    std::string_view _text;
};
