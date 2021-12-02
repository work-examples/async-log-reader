#pragma once

#include "CCharBuffer.h"
#include "CFnMatch.h"
#include "CScanFile.h"

#include <wchar.h> // for size_t, wchar_t


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

    // request next matching line; line may contain '\0' and may end with '\n'; return false on error or EOF
    bool GetNextLine(char* buf, const size_t bufsize, size_t& readBytes);

protected:
    CScanFile   _file;
    CCharBuffer _buffer;

    //bool        _bufferContainsPartialLine = false;
    //bool        _eof = false;
    size_t      _bufferBeginOffset = 0; // first byte which is not returned to a caller yet
    size_t      _bufferEndOffset = 0; // offset after the last byte which is not returned to a caller

    //size_t      _lineBeginOffset = 0;
    //size_t      _lineNextCharOffset = 0;
    //size_t      _bufferEndOffset = 0;

    // size_t      _bufferBeginOffset = 0; // first byte which is not returned to a caller yet
    // size_t      _bufferNoEolOffset = 0; // current position to start searching the EOL?? BOOL???
    //size_t      _bufferEndOffset = 0; // byte after the last byte which is not returned to a caller
    CFnMatch    _lineMatcher;
};
