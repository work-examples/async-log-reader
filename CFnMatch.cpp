#include "CFnMatch.h"


bool CFnMatch::SetPattern(const char* const pattern)
{
    if (pattern == nullptr)
    {
        return false;
    }

    const size_t patternLen = strlen(pattern);
    const bool allocatedOk = this->_pattern.Allocate(patternLen);
    if (!allocatedOk)
    {
        return false;
    }

    memcpy(this->_pattern.ptr, pattern, patternLen);

    return true;
}

bool CFnMatch::FullMatch(std::string_view text)
{
    std::string_view pattern = { this->_pattern.ptr, this->_pattern.size };
    size_t patternPos = 0;
    size_t textPos = 0;
    size_t tempTextPos = 0;
    ptrdiff_t asteriskPos = -1;

    while (textPos < text.size())
    {
        if (patternPos < pattern.size() && pattern[patternPos] == '*')
        {
            asteriskPos = patternPos;
            tempTextPos = textPos;
            ++patternPos;
        }
        else if (patternPos < pattern.size() && (
            pattern[patternPos] == '?' ||
            pattern[patternPos] == text[textPos]
            ))
        {
            ++textPos;
            ++patternPos;
        }
        else if (asteriskPos == -1)
        {
            // Characters do not match or pattern is used up
            // and '*' was not met in the pattern before
            return false;
        }
        else
        {
            // Backtrack and match one more character by '*'
            patternPos = asteriskPos + 1;
            ++tempTextPos;
            textPos = tempTextPos;
        }
    }

    for (; patternPos < pattern.size(); ++patternPos)
    {
        if (pattern[patternPos] != '*')
        {
            return false;
        }
    }

    return true;
}
