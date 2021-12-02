#pragma once

#include <wchar.h> // for size_t

#include "CCharBuffer.h"


class CFnMatch
{
public:
    bool SetFilter(const char* filter);  // set line filter; return false on error

    bool CheckMatch(const char* str, const size_t strLength);

protected:
    CCharBuffer _filter;
    CCharBuffer _memo;
};
