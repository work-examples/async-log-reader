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

    bool CheckMatch(const char* const str, const size_t strLength);

protected:
    size_t      _maxLineLength = 0;
    CCharBuffer _filter;
    CCharBuffer _memo;
};
