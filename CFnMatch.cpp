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

    const bool allocatedOk = this->_filter.Allocate(effectiveLength + 1);
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
    this->_filter.size -= 1;
    this->_filter.ptr[this->_filter.size] = '\0';

    // Allocate memory for Dynamic Programming results storage during future calls to CheckMatch():

    this->_memo.Allocate((this->_maxLineLength + 1) * (effectiveLength + 1));

    return true;
}

#define	EOS	'\0'


bool fnmatch2(const char* pattern, const char* string, const char* stringEnd)
{
    while (true)
    {
        char c = *pattern++;
        switch (c)
        {
        case EOS:
            return string == stringEnd;
        case '?':
            if (string == stringEnd)
                return false;
            ++string;
            break;
        case '*':
            c = *pattern;
            /* Collapse multiple stars. */
            //while (c == '*')
            //    c = *++pattern;

            /* Optimize for pattern with * at end or before /. */
            if (c == EOS)
                return true;

            /* General case, use recursion. */
            for (; string != stringEnd; ++string)
            {
                if (fnmatch2(pattern, string, stringEnd))
                    return true;
            }
            return false;
            /* FALLTHROUGH */
        default:
            if (c != *string)
                return false;
            ++string;
            break;
        }
    }
    /* NOTREACHED */
}


bool CFnMatch::CheckMatch(std::string_view text)
{
    std::string_view pattern = { this->_filter.ptr, this->_filter.size };
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

    //def isMatch(self, s: str, p: str) -> bool:
    //    s_len, p_len = len(s), len(p)
    //    s_idx = p_idx = 0
    //    star_idx = s_tmp_idx = -1
 
    //    while s_idx < s_len:
    //        # If the pattern caracter = string character
    //        # or pattern character = '?'
    //        if p_idx < p_len and p[p_idx] in ['?', s[s_idx]]:
    //            s_idx += 1
    //            p_idx += 1
    //
    //        # If pattern character = '*'
    //        elif p_idx < p_len and p[p_idx] == '*':
    //            # Check the situation
    //            # when '*' matches no characters
    //            star_idx = p_idx
    //            s_tmp_idx = s_idx
    //            p_idx += 1
    //                          
    //        # If pattern character != string character
    //        # or pattern is used up
    //        # and there was no '*' character in pattern 
    //        elif star_idx == -1:
    //            return False
    //                          
    //        # If pattern character != string character
    //        # or pattern is used up
    //        # and there was '*' character in pattern before
    //        else:
    //            # Backtrack: check the situation
    //            # when '*' matches one more character
    //            p_idx = star_idx + 1
    //            s_idx = s_tmp_idx + 1
    //            s_tmp_idx = s_idx
    //    
    //    # The remaining characters in the pattern should all be '*' characters
    //    return all(p[i] == '*' for i in range(p_idx, p_len))

    //if (text.size() > this->_maxLineLength)
    //{
    //    assert(false && "This should never happen if CFnMatch caller has no bugs");
    //    return false;
    //}

    //return fnmatch2(this->_filter.ptr, text.data(), text.data() + text.size());

    ////const size_t memoSize = (this->_filter.size + 1) * (text.size() + 1);
    ////assert(memoSize <= this->_memo.size);
    ////memset(this->_memo.ptr, NoValue, memoSize);

    //this->_text = text;

    //const bool result = this->CalculateMatch(0, 0);
    //return result;
}

bool CFnMatch::CalculateMatch(const size_t filterPos, const size_t textPos)
{
    //assert(filterPos <= this->_filter.size);
    //assert(textPos <= this->_text.size());

    //char& memoResult = this->_memo.ptr[textPos * (this->_filter.size + 1) + filterPos];
    //if (memoResult != NoValue)
    //{
    //    return memoResult;
    //}

    bool result = false;

    if (filterPos == this->_filter.size)
    {
        result = textPos == this->_text.size();
    }
    else
    {
        switch (this->_filter.ptr[filterPos])
        {
        case '*':
            // Optimize for pattern with * at the end
            //if (filterPos + 1 == this->_filter.size)
            //{
            //    //memoResult = 1;
            //    return true;
            //}

            result = (
                textPos < this->_text.size() &&
                this->CalculateMatch(filterPos, textPos + 1)
                ) || this->CalculateMatch(filterPos + 1, textPos);

        case '?':
            result = textPos < this->_text.size() && this->CalculateMatch(filterPos + 1, textPos + 1);
            break;

        default:
            const bool currentMatch = textPos < this->_text.size() && this->_filter.ptr[filterPos] == this->_text[textPos];
            result = currentMatch && this->CalculateMatch(filterPos + 1, textPos + 1);
            break;
        }

        //if (this->_filter.ptr[filterPos] == '*')
        //{
        //    // Optimize for pattern with * at the end
        //    if (filterPos + 1 == this->_filter.size)
        //    {
        //        //memoResult = 1;
        //        return true;
        //    }

        //    result = this->CalculateMatch(filterPos + 1, textPos) || (
        //        textPos < this->_text.size() && 
        //        this->CalculateMatch(filterPos, textPos + 1)
        //        );
        //}
        //else
        //{
        //    const bool currentMatch = textPos < this->_text.size() && (
        //        this->_filter.ptr[filterPos] == this->_text[textPos] ||
        //        this->_filter.ptr[filterPos] == '?'
        //        );
        //    result = currentMatch && this->CalculateMatch(filterPos + 1, textPos + 1);
        //}
    }

    //memoResult = static_cast<char>(result);
    return result;
}

// https://opensource.apple.com/source/Libc/Libc-167/gen.subproj/fnmatch.c.auto.html
//#include <fnmatch.h>
//#include <string.h>

#define	EOS	'\0'


bool fnmatch(const char* pattern, const char* string)
{
    while(true)
    {
        char c = *pattern++;
        switch (c)
        {
        case EOS:
            return *string == EOS;
        case '?':
            if (*string == EOS)
                return false;
            ++string;
            break;
        case '*':
            c = *pattern;
            /* Collapse multiple stars. */
            while (c == '*')
                c = *++pattern;

            /* Optimize for pattern with * at end or before /. */
            if (c == EOS)
                return true;

            /* General case, use recursion. */
            for (; *string != EOS; ++string)
            {
                if (fnmatch(pattern, string))
                    return true;
            }
            return false;
            /* FALLTHROUGH */
        default:
            if (c != *string++)
                return false;
            break;
        }
    }
    /* NOTREACHED */
}

