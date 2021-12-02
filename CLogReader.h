#pragma once

#include <wchar.h> // for size_t, wchar_t

#include "CCharBuffer.h"
#include "CFnMatch.h"
#include "CScanFile.h"


class CLogReader final
{
public:
    CLogReader();

    // open file; return false on error
    bool Open(const wchar_t* const filename);
    // close file
    void Close();

    // set line filter; return false on error
    bool SetFilter(const char* const filter);

    // request next matching line; line may contain '\0' and may end with '\n'; return false on error
    bool GetNextLine(char* buf, const size_t bufsize, size_t& readBytes);

protected:
    CScanFile   _file;
    CCharBuffer _buffer;
    size_t      _bufferBeginReadOffset = 0;
    size_t      _bufferEndReadOffset = 0;
    CFnMatch    _lineMatcher;
};
