#pragma once

#include <wchar.h> // for size_t

#include "CCharBuffer.h"


class CFnMatch
{
public:
    CFnMatch(const size_t maxLineLength);

    bool SetFilter(const char* const filter);  // set line filter; return false on error

    bool CheckMatch(const char* const str, const size_t strLength);

protected:
    size_t      _maxLineLength = 0;
    CCharBuffer _filter;
    CCharBuffer _memo;
};
