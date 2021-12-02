#include "CFnMatch.h"

#include <assert.h>


CFnMatch::CFnMatch(const size_t maxLineLength): _maxLineLength(maxLineLength)
{
}

bool CFnMatch::SetFilter(const char* const filter)
{
    if (filter == nullptr)
    {
        return false;
    }

    // Copy filter string and compress consecutive asterisks to simplify numerous filter applies:
    // "***ABC*****DEF**" => "*ABC*DEF*"

    size_t effectiveLength = 0;
    
    for (const char* p = filter; p[0] != '\0'; p += 1)
    {
        if (p[0] == '*' and p[1] == '*')
        {
            // Glue consecutive asterisks together by skipping all asterisks except the last one in sequence
            continue;
        }
        effectiveLength += 1;
    }

    const bool allocatedOk = this->_filter.Allocate(effectiveLength);
    if (!allocatedOk)
    {
        return false;
    }

    size_t destOffset = 0;

    for (const char* p = filter; p[0] != '\0'; p += 1)
    {
        if (p[0] == '*' and p[1] == '*')
        {
            continue;
        }
        this->_filter.ptr[destOffset] = p[0];
        destOffset += 1;
    }
    assert(destOffset == effectiveLength);

    // Allocate memory for Dynamic Programming results storage during future calls to CheckMatch():

    this->_memo.Allocate(this->_maxLineLength * effectiveLength);

    return true;
}


bool CFnMatch::CheckMatch(const char* const str, const size_t strLength)
{
    // TODO: IMPLEMENT!!!!!!!!!!!!!!!
    return true;
}
