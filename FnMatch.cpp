#include "FnMatch.h"

#include <assert.h>


#if 1
// Implementation with optimized speed (6.5x times faster in my dataset by than original naive implementation)

__declspec(noinline) // noinline is added to help CPU profiling in release version
bool CFnMatch::Match(const std::string_view text, const std::string_view pattern)
{
    const char* pText = text.data();
    const char* const pTextEnd = pText + text.size();
    const char* pPattern = pattern.data();
    const char* const pPatternEnd = pPattern + pattern.size();

    const char* pPatternAfterAsterisk = nullptr;
    const char* pAsteriskMatchEnd = nullptr;

    while (pText < pTextEnd)
    {
        if (pPattern < pPatternEnd && *pPattern == '*')
        {
            pPatternAfterAsterisk = ++pPattern;
            pAsteriskMatchEnd = pText;

            // OPTIONAL: Speedup the loop (optional block with quick forward lookup):
            {
                while (pPattern < pPatternEnd && *pPattern == '*')
                {
                    ++pPattern;
                    ++pPatternAfterAsterisk;
                }
                if (pPattern < pPatternEnd && *pPattern != '?')
                {
                    const char* const p = static_cast<const char*>(memchr(pText, *pPattern, pTextEnd - pText));
                    if (p == nullptr)
                    {
                        return false;
                    }
                    pAsteriskMatchEnd = p;
                    pText = p + 1;
                    ++pPattern;
                }
            }
            continue;
        }
        else if (pPattern < pPatternEnd && (
            *pPattern == '?' ||
            *pPattern == *pText
            ))
        {
            ++pText;
            ++pPattern;
            continue;
        }
        else if (pPatternAfterAsterisk == nullptr)
        {
            // Characters do not match or pattern is used up
            // and '*' was not met in the pattern before
            return false;
        }
        else
        {
            // Backtrack and match one more character by '*'
            pPattern = pPatternAfterAsterisk;
            pText = ++pAsteriskMatchEnd;

            // OPTIONAL: Speedup the loop (optional block with quick forward lookup):
            {
                if (pPattern < pPatternEnd && *pPattern != '?')
                {
                    assert(*pPattern != '*' && "This is guaranteed by the speedup block above");
                    const char* const p = static_cast<const char*>(memchr(pText, *pPattern, pTextEnd - pText));
                    if (p == nullptr)
                    {
                        return false;
                    }
                    pAsteriskMatchEnd = p;
                    pText = p + 1;
                    ++pPattern;
                }
            }
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

#else
// Original implementation

__declspec(noinline) // noinline is added to help CPU profiling in release version
bool CFnMatch::Match(const std::string_view text, const std::string_view pattern)
{
    size_t patternPos = 0;
    size_t textPos = 0;
    size_t asteriskMatchEndTextPos = 0;
    ptrdiff_t asteriskPatternPos = -1;

    while (textPos < text.size())
    {
        if (patternPos < pattern.size() && pattern[patternPos] == '*')
        {
            asteriskPatternPos = patternPos;
            asteriskMatchEndTextPos = textPos;
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
        else if (asteriskPatternPos == -1)
        {
            // Characters do not match or pattern is used up
            // and '*' was not met in the pattern before
            return false;
        }
        else
        {
            // Backtrack and match one more character by '*'
            patternPos = asteriskPatternPos + 1;
            ++asteriskMatchEndTextPos;
            textPos = asteriskMatchEndTextPos;
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
