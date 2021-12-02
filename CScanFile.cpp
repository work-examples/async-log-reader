#include "CScanFile.h"

#include <algorithm>

#include <windows.h>


CScanFile::~CScanFile()
{
    this->Close();
}

bool CScanFile::Open(const wchar_t* const filename)
{
    if (filename == nullptr || this->_file != nullptr)
    {
        return false;
    }

    const DWORD dwDesiredAccess = FILE_READ_DATA; // minimal required rights
    const DWORD dwShareMode = FILE_SHARE_READ; // allow parallel reading. And do not allow appending to log. Algorithm will not work correctly in this case.
    const DWORD dwCreationDisposition = OPEN_EXISTING;
    const DWORD dwFlagsAndAttributes = FILE_FLAG_SEQUENTIAL_SCAN; // read comment below

    // FILE_FLAG_SEQUENTIAL_SCAN gives a cache speed optimization for pattern when file is read once from the beginning to the end
    //
    // We can also use async file reading, but it will complicate the code and has its own trade offs related with additional regular calls to kernel mode.
    // I'm sure FILE_FLAG_SEQUENTIAL_SCAN will force Windows to act with similar performance to what we could get using Async IO.
    // However I did no performance checks for this guess.

    this->_file = CreateFileW(filename, dwDesiredAccess, dwShareMode, nullptr, dwCreationDisposition, dwFlagsAndAttributes, nullptr);

    if (this->_file == INVALID_HANDLE_VALUE)
    {
        this->_file = nullptr;
        return false;
    }

    return true;
}

void CScanFile::Close()
{
    if (this->_file != nullptr)
    {
        CloseHandle(this->_file);
        this->_file = nullptr;
    }
}

bool CScanFile::Read(char* buffer, const size_t bufferLength, size_t& readBytes)
{
    if (this->_file == nullptr || buffer == nullptr)
    {
        return false;
    }

    if (bufferLength == 0)
    {
        readBytes = 0;
        return true;
    }

    const DWORD usedBufferLength = static_cast<DWORD>(min(bufferLength, MAXDWORD));
    DWORD numberOfBytesRead = 0;

    const BOOL succeeded = ReadFile(this->_file, buffer, usedBufferLength, &numberOfBytesRead, nullptr);
    readBytes = numberOfBytesRead;

    return !!succeeded;
}
