#pragma once

#include <optional> // this is STL, but it does not need exceptions
#include <string_view> // this is STL, but it does not need exceptions

#include <wchar.h> // for size_t, wchar_t

#include <windows.h>


class CScanFile
{
public:
    ~CScanFile();

    bool Open(const wchar_t* const filename, const bool asyncMode);
    void Close();

    std::optional<std::string_view> MapToMemory();

    bool Read(char* const buffer, const size_t bufferLength, size_t& readBytes);

    // Current limitation: only one async operation can be in progress.
    // You will need to have multiple OVERLAPPED structures and multiple events to handle few requests simultaneously.
    bool AsyncReadStart(char* const buffer, const size_t bufferLength);
    bool AsyncReadWait(size_t& readBytes);

protected:
    HANDLE        _hFile = nullptr;
    // For memory mapping:
    HANDLE        _hFileMapping = nullptr;
    void*         _pViewOfFile = nullptr;
    // For async IO:
    HANDLE        _hEvent = nullptr;
    LARGE_INTEGER _fileOffset = {};
    OVERLAPPED    _overlapped = {};
    bool          _operationInProgress = false;
};
