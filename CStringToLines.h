#pragma once

#include <string_view> // this is STL, but it does not need exceptions


class CStringToLines
{
public:
    // Return first line from 'text' including CRLF/LF; shrink 'text' by removing returned buffer range;
    // Return empty view in case it cannot find '\n' in the 'text'.
    static std::string_view PopFirstLine(std::string_view& text)
    {
        const size_t eolOffset = text.find('\n');
        if (eolOffset == text.npos)
        {
            return {};
        }
        else
        {
            const std::string_view line = text.substr(0, eolOffset + 1);
            text.remove_prefix(eolOffset + 1);
            return line;
        }
    }
};
