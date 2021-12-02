#pragma once

#include <wchar.h> // for size_t, wchar_t


class CScanFile
{
public:
    ~CScanFile();

    bool Open(const wchar_t* const filename);
    void Close();
    bool Read(char* buffer, const size_t bufferLength, size_t& readBytes);

protected:
    void* _file = nullptr; // I'm not using HANDLE here to avoid including windows.h in header and reduce compilation speed without PCH
};
