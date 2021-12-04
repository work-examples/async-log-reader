#pragma once

#include <wchar.h> // for size_t, wchar_t

#include <windows.h>


class CScanFile
{
public:
    ~CScanFile();

    bool Open(const wchar_t* const filename);
    void Close();
    bool Read(char* buffer, const size_t bufferLength, size_t& readBytes);

protected:
    HANDLE _hFile = nullptr;
    HANDLE _hEvent = nullptr;
    LARGE_INTEGER _fileOffset = {};
    OVERLAPPED _overlapped = {};
};
