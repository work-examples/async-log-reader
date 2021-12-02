#pragma once

#include <wchar.h> // for size_t, wchar_t

#include "CCharBuffer.h"
#include "CFnMatch.h"
#include "CScanFile.h"


class CLogReader final
{
public:
    CLogReader();

    bool Open(const wchar_t* filename);  // open file; return false on error
    void Close();                        // close file

    bool SetFilter(const char* filter);  // set line filter; return false on error
    bool GetNextLine(char* buf, const size_t bufsize); // request next matching line; return false on error

protected:
    CScanFile   _file;
    CCharBuffer _readBuffer;
    size_t      _readBufferUsedBytes = 0;
    CFnMatch    _lineMatcher;
};
