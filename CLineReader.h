#pragma once

#include "CCharBuffer.h"

#include <functional>  // this is STL, but it does not need exceptions
#include <optional>    // this is STL, but it does not need exceptions
#include <string_view> // this is STL, but it does not need exceptions

#include <wchar.h> // for size_t


class CLineReader
{
public:
    using ReadDataFunc = bool(char* buffer, const size_t bufferLength, size_t& readBytes);

    CLineReader();

    bool Setup(std::function<ReadDataFunc> readData);

    // request next matching line; line may contain '\0' and may end with '\n'; return false on error or EOF
    std::optional<std::string_view> GetNextLine();

protected:
    std::function<ReadDataFunc> _funcReadData;

    CCharBuffer      _buffer; // it is filled with at least ReadBlockSize bytes all the time except last chunk of data
    std::string_view _bufferData; // filled part of the buffer
};
