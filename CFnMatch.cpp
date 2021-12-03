#include "CFnMatch.h"

#include <assert.h>


namespace
{
    constexpr char NoValue = 3;
}


CFnMatch::CFnMatch(const size_t maxLineLength) : _maxLineLength(maxLineLength)
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
        ++effectiveLength;
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
        ++destOffset;
    }
    assert(destOffset == effectiveLength);

    // Allocate memory for Dynamic Programming results storage during future calls to CheckMatch():

    this->_memo.Allocate((this->_maxLineLength + 1) * (effectiveLength + 1));

    return true;
}

bool CFnMatch::CheckMatch(std::string_view text)
{
    if (text.size() > this->_maxLineLength)
    {
        assert(false && "This should never happen if CFnMatch caller has no bugs");
        return false;
    }

    const size_t memoSize = (this->_filter.size + 1) * (text.size() + 1);
    assert(memoSize <= this->_memo.size);
    memset(this->_memo.ptr, NoValue, memoSize);

    this->_text = text;

    const bool result = this->CalculateMatch(0, 0);
    return result;
}

bool CFnMatch::CalculateMatch(const size_t filterPos, const size_t textPos)
{
    char& memoResult = this->_memo.ptr[textPos * (this->_filter.size + 1) + filterPos];
    if (memoResult != NoValue)
    {
        return memoResult;
    }

    bool result = false;

    if (filterPos == this->_filter.size)
    {
        result = textPos == this->_text.size();
    }
    else
    {
        if (this->_filter.ptr[filterPos] == '*')
        {
            result = this->CalculateMatch(filterPos + 1, textPos) || (
                textPos < this->_text.size() && 
                this->CalculateMatch(filterPos, textPos + 1)
                );
        }
        else
        {
            const bool currentMatch = textPos < this->_text.size() && (
                this->_filter.ptr[filterPos] == this->_text[textPos] ||
                this->_filter.ptr[filterPos] == '?'
                );
            result = currentMatch && this->CalculateMatch(filterPos + 1, textPos + 1);
        }
    }

    memoResult = static_cast<char>(result);
    return result;
}
