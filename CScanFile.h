#pragma once

#include <wchar.h> // for size_t, wchar_t

#include <windows.h>


class CScanFile
{
public:
    ~CScanFile();

    bool Open(const wchar_t* const filename, const bool asyncMode);
    void Close();

    bool Read(char* buffer, const size_t bufferLength, size_t& readBytes);

    // Current limitation: only one async operation can be in progress.
    // You will need to have multiple OVERLAPPED structures and multiple events to handle few requests simultaneously.
    bool AsyncReadStart(char* buffer, const size_t bufferLength);
    bool AsyncReadWait(size_t& readBytes);

protected:
    HANDLE _hFile = nullptr;
    HANDLE _hEvent = nullptr;
    LARGE_INTEGER _fileOffset = {};
    OVERLAPPED _overlapped = {};
    bool _operationInProgress = false;
};
