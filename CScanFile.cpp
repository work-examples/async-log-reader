#include "CScanFile.h"

#include <windows.h>

#include <algorithm>


CScanFile::~CScanFile()
{
    this->Close();
}

bool CScanFile::Open(const wchar_t* filename)
{
    if (filename == nullptr || this->_file != nullptr)
    {
        return false;
    }

    const DWORD dwDesiredAccess = FILE_READ_DATA;
    const DWORD dwShareMode = FILE_SHARE_READ;
    const DWORD dwCreationDisposition = OPEN_EXISTING;
    const DWORD dwFlagsAndAttributes = FILE_FLAG_SEQUENTIAL_SCAN;  // cache speed optimization for pattern when file is read once from the beginning to the end

    this->_file = CreateFileW(filename, dwDesiredAccess, dwShareMode, nullptr, 0, dwFlagsAndAttributes, nullptr);

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
