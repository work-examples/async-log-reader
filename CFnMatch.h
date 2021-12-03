#pragma once

#include "CCharBuffer.h"

#include <string_view> // this is STL, but it does not need exceptions

#include <wchar.h> // for size_t


class CFnMatch
{
public:
    CFnMatch(const size_t maxLineLength);

    bool SetFilter(const char* const filter);  // set line filter; return false on error

    std::string_view GetFilter()
    {
        return { this->_filter.ptr, this->_filter.size };
    }

    // Single threaded! Don't try to matching few strings at the same time using the same CFnMatch object!
    // This is done to reduce stack usage and to mostly to avoid memory allocation for _memo during every call.
    bool CheckMatch(std::string_view text);

protected:
    bool CalculateMatch(const size_t filterPos, const size_t textPos); // returns 0/1 (bool)

protected:
    size_t           _maxLineLength = 0;
    CCharBuffer      _filter;
    CCharBuffer      _memo;
    std::string_view _text;
};
