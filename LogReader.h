#pragma once

#include "CharBuffer.h"
#include "FnMatch.h"
#include "LineReader.h"

#include <optional> // this is STL, but it does not need exceptions
#include <string_view> // this is STL, but it does not need exceptions

#include <wchar.h> // for size_t, wchar_t


class CLogReader final
{
public:
    // open file; return false on error
    // Supported encoding for file data: utf-8, any single-byte encodings (with ASCII backward support)
    // Supported line endings: CRLF, LF (\r\n, \n)
    // '\0' is supported inside of log lines. However SetFilter() does not support using it in filter. And one of GetNextLine() methods does not support it too.
    bool Open(const wchar_t* const filename);

    // close file
    void Close();

    // set line filter; return false on error
    bool SetFilter(const char* const filter);

    // request next matching line; line may contain '\0' and may end with CRLF or LF; return false on error or EOF
    std::optional<std::string_view> GetNextLine();

    // NOTE: I would prefer to drop next method because it needs additional memory copy and does not support null characters inside of the line.
    //       I'm keeping the method only for compatibility with original requirements.
    bool GetNextLine(char* buf, const size_t bufsize)
    {
        if (buf == nullptr)
        {
            return false;
        }
        const auto line = this->GetNextLine();
        if (!line || line->size() >= bufsize)
        {
            return false;
        }
        memcpy(buf, line->data(), line->size());
        buf[line->size()] = '\0';
        return true;
    }

protected:
#if 0
#if 1
    CSyncLineReader    _lineReader;
#else
    CMappingLineReader _lineReader;
#endif
#else
#if 0
    CAsyncLineReader    _lineReader;
#else
    CSpinlockLineReader _lineReader;
#endif
#endif
    CCharBuffer        _pattern;
    CFnMatch           _lineMatcher;
};
