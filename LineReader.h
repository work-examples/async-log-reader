#pragma once

#include "CharBuffer.h"
#include "ScanFile.h"

#include <optional>    // this is STL, but it does not need exceptions
#include <string_view> // this is STL, but it does not need exceptions

#include <wchar.h> // for size_t


class CSyncLineReader
{
public:
    CSyncLineReader();

    bool Open(const wchar_t* const filename);
    void Close();

    // request next matching line; line may contain '\0' and may end with '\n'; return false on error or EOF
    std::optional<std::string_view> GetNextLine();

protected:
    CScanFile        _file;
    // Buffer structure: [    rest_of_previousline|data_read_from_file  ]
    //                   [ len = MaxLogLineLength | len = ReadChunkSize ]
    CCharBuffer      _buffer;
    std::string_view _bufferData; // filled part of the buffer
};
