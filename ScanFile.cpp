#include "ScanFile.h"

#include <assert.h>

#include <algorithm>

#include <windows.h>


CScanFile::~CScanFile()
{
    this->Close();
}

bool CScanFile::Open(const wchar_t* const filename, const bool asyncMode)
{
    if (filename == nullptr || this->_hFile != nullptr)
    {
        return false;
    }

    assert(this->_hEvent == nullptr);

    const DWORD dwDesiredAccess = FILE_READ_DATA | FILE_READ_ATTRIBUTES; // minimal required rights
    // FILE_READ_ATTRIBUTES is needed to get file size for mapping file to memory
    const DWORD dwShareMode = FILE_SHARE_READ; // allow parallel reading. And do not allow appending to log. Algorithm will not work correctly in this case.
    const DWORD dwCreationDisposition = OPEN_EXISTING;
    const DWORD dwFlagsAndAttributes = FILE_FLAG_SEQUENTIAL_SCAN | (asyncMode ? FILE_FLAG_OVERLAPPED : 0); // read the comment below

    // FILE_FLAG_SEQUENTIAL_SCAN gives a cache speed optimization for pattern when file is read once from the beginning to the end
    // According to my tests FILE_FLAG_SEQUENTIAL_SCAN does no measurable impact but I would prefer to keep it here.

    this->_hFile = CreateFileW(filename, dwDesiredAccess, dwShareMode, nullptr, dwCreationDisposition, dwFlagsAndAttributes, nullptr);

    if (this->_hFile == INVALID_HANDLE_VALUE)
    {
        this->_hFile = nullptr;
        return false;
    }

    this->_hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);

    if (this->_hEvent == nullptr)
    {
        CloseHandle(this->_hFile);
        this->_hFile = nullptr;
        return false;
    }

    this->_overlapped.hEvent = this->_hEvent;
    this->_fileOffset.QuadPart = 0;
    this->_operationInProgress = false;

    return true;
}

void CScanFile::Close()
{
    if (this->_pViewOfFile != nullptr)
    {
        UnmapViewOfFile(this->_pViewOfFile);
        this->_pViewOfFile = nullptr;
    }

    if (this->_hFileMapping != nullptr)
    {
        CloseHandle(this->_hFileMapping);
        this->_hFileMapping = nullptr;
    }

    if (this->_hFile != nullptr)
    {
        CloseHandle(this->_hFile);
        this->_hFile = nullptr;
    }

    if (this->_hEvent != nullptr)
    {
        CloseHandle(this->_hEvent);
        this->_hEvent = nullptr;
    }

    this->_operationInProgress = false;
}

std::optional<std::string_view> CScanFile::MapToMemory()
{
    if (this->_hFile == nullptr || this->_hFileMapping != nullptr)
    {
        return {};
    }
    assert(this->_pViewOfFile == nullptr);

    LARGE_INTEGER fileSize = {};
    const bool gotSizeOk = GetFileSizeEx(this->_hFile, &fileSize);
    if (!gotSizeOk)
    {
        return {};
    }

    if (fileSize.QuadPart == 0)
    {
        // MSDN: An attempt to map a file with a length of 0 (zero) fails with an error code of ERROR_FILE_INVALID.
        // Applications should test for files with a length of 0 (zero) and reject those files.
        return std::string_view();
    }

    const size_t fileSizeAsSizeT = static_cast<size_t>(fileSize.QuadPart);
    if (fileSize.QuadPart > SIZE_MAX)
    {
        // 32 bit application can have problems with mapping of big files into address space
        return {};
    }

    this->_hFileMapping = CreateFileMappingW(this->_hFile, nullptr, PAGE_READONLY, fileSize.HighPart, fileSize.LowPart, nullptr);
    if (this->_hFileMapping == nullptr)
    {
        return {};
    }

    this->_pViewOfFile = MapViewOfFile(this->_hFileMapping, FILE_MAP_READ, 0, 0, fileSizeAsSizeT);
    if (this->_pViewOfFile == nullptr)
    {
        CloseHandle(this->_hFileMapping);
        this->_hFileMapping = nullptr;
        return {};
    }

    return std::string_view(static_cast<const char*>(this->_pViewOfFile), fileSizeAsSizeT);
}

__declspec(noinline) // noinline is added to help CPU profiling in release version
bool CScanFile::Read(char* const buffer, const size_t bufferLength, size_t& readBytes)
{
    if (this->_hFile == nullptr || buffer == nullptr)
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

    const bool succeeded = !!ReadFile(this->_hFile, buffer, usedBufferLength, &numberOfBytesRead, nullptr);
    readBytes = numberOfBytesRead;

    return !!succeeded;
}

__declspec(noinline) // noinline is added to help CPU profiling in release version
bool CScanFile::AsyncReadStart(char* const buffer, const size_t bufferLength)
{
    if (this->_hFile == nullptr || buffer == nullptr || this->_operationInProgress)
    {
        return false;
    }
    assert(this->_hEvent != nullptr);

    const DWORD usedBufferLength = static_cast<DWORD>(min(bufferLength, MAXDWORD));

    this->_overlapped.Offset = this->_fileOffset.LowPart;
    this->_overlapped.OffsetHigh = this->_fileOffset.HighPart;

    const bool readOk = !!ReadFile(this->_hFile, buffer, usedBufferLength, nullptr, &this->_overlapped);
    if (!readOk && GetLastError() != ERROR_IO_PENDING)
    {
        return false;
    }

    this->_operationInProgress = true;

    return true;
}

__declspec(noinline) // noinline is added to help CPU profiling in release version
bool CScanFile::AsyncReadWait(size_t& readBytes)
{
    if (!this->_operationInProgress)
    {
        return false;
    }
    assert(this->_hFile != nullptr);
    assert(this->_hEvent != nullptr);

    DWORD numberOfBytesRead = 0;

    const bool overlappedOk = !!GetOverlappedResult(this->_hFile, &this->_overlapped, &numberOfBytesRead, TRUE);
    if (!overlappedOk)
    {
        this->_operationInProgress = false; // I'm not sure this is correct
        if (GetLastError() == ERROR_HANDLE_EOF)
        {
            assert(numberOfBytesRead == 0);
            // Emulate usual ReadFile() logic when reading at the end succeeds with zero bytes read
            readBytes = 0;
            return true;
        }
        return false;
    }

    this->_fileOffset.QuadPart += numberOfBytesRead;
    this->_operationInProgress = false;

    readBytes = numberOfBytesRead;

    return true;
}
