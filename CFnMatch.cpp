#include "CFnMatch.h"

bool CFnMatch::Match(const std::string_view text, const std::string_view pattern)
{
    const char* pText = text.data();
    const char* const pTextEnd = pText + text.size();
    const char* pPattern = pattern.data();
    const char* const pPatternEnd = pPattern + pattern.size();

    const char* pAsterisk = nullptr;
    const char* pTempText = nullptr;

    while (pText < pTextEnd)
    {
        if (pPattern < pPatternEnd && *pPattern == '*')
        {
            pAsterisk = pPattern++;
            pTempText = pText;
        }
        else if (pPattern < pPatternEnd && (
            *pPattern == '?' ||
            *pPattern == *pText
            ))
        {
            ++pText;
            ++pPattern;
        }
        else if (pAsterisk == nullptr)
        {
            // Characters do not match or pattern is used up
            // and '*' was not met in the pattern before
            return false;
        }
        else
        {
            // Backtrack and match one more character by '*'
            pPattern = pAsterisk + 1;
            pText = ++pTempText;
        }
    }

    for (; pPattern < pPatternEnd; ++pPattern)
    {
        if (*pPattern != '*')
        {
            return false;
        }
    }

    return true;
}

#if 0
bool CFnMatch::Match(const std::string_view text, const std::string_view pattern)
{
    size_t patternPos = 0;
    size_t textPos = 0;
    size_t tempTextPos = 0;
    ptrdiff_t asteriskPos = -1;
    const char* const pText = text.data();
    const char* const pPattern = pattern.data();
    const size_t textLen = text.size();
    const size_t patternLen = pattern.size();

    while (textPos < textLen)
    {
        if (patternPos < patternLen)
        {
            const char patternChar = pPattern[patternPos];
            switch (patternChar)
            {
            case '*':
                asteriskPos = patternPos;
                tempTextPos = textPos;
                ++patternPos;
                continue;
            case '?':
                ++textPos;
                ++patternPos;
                continue;
            default:
                if (patternChar == pText[textPos])
                {
                    ++textPos;
                    ++patternPos;
                    continue;
                }
            }
        }

        if (asteriskPos == -1)
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

    if (pattern.find_first_not_of('*', patternPos) != pattern.npos)
    {
        return false;
    }

    return true;
}
#endif

#if 0
__declspec(noinline) bool CFnMatch::Match(const std::string_view text, const std::string_view pattern)
{
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

    // Rest of the pattern can contain only asterisks
    if (pattern.find_first_not_of('*', patternPos) != pattern.npos)
    {
        return false;
    }

    return true;
}
#endif
