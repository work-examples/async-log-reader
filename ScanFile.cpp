#include "ScanFile.h"

#include <assert.h>

#include <algorithm>
#include <atomic>       // for std::atomic_thread_fence

#include <windows.h>

namespace
{
    // This is a dirty hack for speedup inter-thread communication on hyperthreading CPUs
    // We could choose any other paid of logical CPUs based on the same physical CPU and sharing the same cache
    // I'm sure it will give no effect in the current task because we are switching too rarely between threads.
    const int PreferedCpuForMainThread = 0;
    const int PreferedCpuForWorkerThread = PreferedCpuForMainThread + 1;
    static_assert(PreferedCpuForMainThread % 2 == 0);
    const DWORD ThreadPriority = THREAD_PRIORITY_TIME_CRITICAL;
#   define ENABLE_THREAD_PREFERRED_AFFINITY 0
#   define ENABLE_THREAD_FIXED_AFFINITY     0
#   define ENABLE_THREAD_PRIORITY           0
}


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
    assert(this->_hAsyncEvent == nullptr);

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

    // Init data for async IO:
    this->_hAsyncEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);

    if (this->_hAsyncEvent == nullptr)
    {
        CloseHandle(this->_hFile);
        this->_hFile = nullptr;
        return false;
    }

    this->_asyncOverlapped.hEvent = this->_hAsyncEvent;
    this->_asyncFileOffset.QuadPart = 0;
    this->_asyncOperationInProgress = false;

    return true;
}

void CScanFile::Close()
{
    this->LockFreecClean();

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

    if (this->_hAsyncEvent != nullptr)
    {
        CloseHandle(this->_hAsyncEvent);
        this->_hAsyncEvent = nullptr;
    }

    this->_asyncOperationInProgress = false;
}

//////////////////////////////////////////////////////////////////////////
/// Implementation of mapping file to memory
//////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////
/// Implementation of synchronous file API
//////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////
/// Implementation of Asynchronous file API
//////////////////////////////////////////////////////////////////////////

__declspec(noinline) // noinline is added to help CPU profiling in release version
bool CScanFile::AsyncReadStart(char* const buffer, const size_t bufferLength)
{
    if (this->_hFile == nullptr || buffer == nullptr || this->_asyncOperationInProgress)
    {
        return false;
    }
    assert(this->_hAsyncEvent != nullptr);

    const DWORD usedBufferLength = static_cast<DWORD>(min(bufferLength, MAXDWORD));

    this->_asyncOverlapped.Offset = this->_asyncFileOffset.LowPart;
    this->_asyncOverlapped.OffsetHigh = this->_asyncFileOffset.HighPart;

    const bool readOk = !!ReadFile(this->_hFile, buffer, usedBufferLength, nullptr, &this->_asyncOverlapped);
    if (!readOk && GetLastError() != ERROR_IO_PENDING)
    {
        return false;
    }

    this->_asyncOperationInProgress = true;

    return true;
}

__declspec(noinline) // noinline is added to help CPU profiling in release version
bool CScanFile::AsyncReadWait(size_t& readBytes)
{
    if (!this->_asyncOperationInProgress)
    {
        return false;
    }
    assert(this->_hFile != nullptr);
    assert(this->_hAsyncEvent != nullptr);

    DWORD numberOfBytesRead = 0;

    const bool overlappedOk = !!GetOverlappedResult(this->_hFile, &this->_asyncOverlapped, &numberOfBytesRead, TRUE);
    if (!overlappedOk)
    {
        this->_asyncOperationInProgress = false; // I'm not sure this is correct
        if (GetLastError() == ERROR_HANDLE_EOF)
        {
            assert(numberOfBytesRead == 0);
            // Emulate usual ReadFile() logic when reading at the end succeeds with zero bytes read
            readBytes = 0;
            return true;
        }
        return false;
    }

    this->_asyncFileOffset.QuadPart += numberOfBytesRead;
    this->_asyncOperationInProgress = false;

    readBytes = numberOfBytesRead;

    return true;
}

//////////////////////////////////////////////////////////////////////////
/// Implementation of file API executed in a separate thread with the help of spinlocks
//////////////////////////////////////////////////////////////////////////

bool CScanFile::LockFreecInit()
{
    if (this->_hThread != nullptr)
    {
        return false;
    }

    this->_threadFinishSpinlock = false;
    this->_threadOperationReadStartSpinlock = false;
    this->_threadOperationReadCompletedSpinlock = false;
    this->_pThreadReadBuffer = nullptr;
    this->_threadReadBufferSize = 0;
    this->_threadActuallyReadBytes = 0;
    this->_threadReadSucceeded = false;
    std::atomic_thread_fence(std::memory_order_release);

    // This is a dirty hack for speedup inter-thread communication on hyperthreading CPUs
#if ENABLE_THREAD_PREFERRED_AFFINITY
    SetThreadIdealProcessor(GetCurrentThread(), PreferedCpuForMainThread);
#endif
#if ENABLE_THREAD_FIXED_AFFINITY
    SetThreadAffinityMask(GetCurrentThread(), 1 << PreferedCpuForMainThread);
#endif
#if ENABLE_THREAD_PRIORITY
    SetThreadPriority(GetCurrentThread(), ThreadPriority);
#endif

    using ThreadProcType = unsigned __stdcall(void*);
    ThreadProcType* const threadProc = [](void* p) -> unsigned
    {
        // This is a dirty hack for speedup inter-thread communication on hyperthreading CPUs
#if ENABLE_THREAD_PREFERRED_AFFINITY
        SetThreadIdealProcessor(GetCurrentThread(), PreferedCpuForWorkerThread);
#endif
#if ENABLE_THREAD_FIXED_AFFINITY
        SetThreadAffinityMask(GetCurrentThread(), 1 << PreferedCpuForWorkerThread);
#endif
#if ENABLE_THREAD_PRIORITY
        SetThreadPriority(GetCurrentThread(), ThreadPriority);
#endif

        CScanFile* const that = static_cast<CScanFile*>(p);
        that->LockFreeThreadProc();
        _endthreadex(0);
        return 0;
    };

    unsigned threadID = 0;
    this->_hThread = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, threadProc, this, 0, &threadID));
    if (this->_hThread == nullptr)
    {
        return false;
    }

    return true;
}

void CScanFile::LockFreecClean()
{
    if (this->_hThread != nullptr)
    {
        this->_threadFinishSpinlock = true;
        WaitForSingleObject(this->_hThread, INFINITE); // ignore return value in this case
        this->_hThread = nullptr;
    }
}

__declspec(noinline) // noinline is added to help CPU profiling in release version
bool CScanFile::LockFreeReadStart(char* const buffer, const size_t bufferLength)
{
    if (this->_hThread == nullptr || this->_threadOperationInProgress)
    {
        return false;
    }

    this->_threadOperationInProgress = true;
    this->_threadOperationReadCompletedSpinlock = false;
    this->_pThreadReadBuffer = buffer;
    this->_threadReadBufferSize = bufferLength;
    this->_threadActuallyReadBytes = 0;
    this->_threadReadSucceeded = false;
    std::atomic_thread_fence(std::memory_order_release);

    this->_threadOperationReadStartSpinlock = true;
    std::atomic_thread_fence(std::memory_order_release);

    return true;
}

__declspec(noinline) // noinline is added to help CPU profiling in release version
bool CScanFile::LockFreeReadWait(size_t& readBytes)
{
    if (this->_hThread == nullptr || !this->_threadOperationInProgress)
    {
        return false;
    }

    this->_threadOperationInProgress = false;
    std::atomic_thread_fence(std::memory_order_release);

    std::atomic_thread_fence(std::memory_order_acquire);
    while (!this->_threadOperationReadCompletedSpinlock)
    {
        // nothing
        std::atomic_thread_fence(std::memory_order_acquire);
    }

    std::atomic_thread_fence(std::memory_order_acquire);
    readBytes = this->_threadActuallyReadBytes;
    if (!this->_threadReadSucceeded)
    {
        return false;
    }

    return true;
}

void CScanFile::LockFreeThreadProc()
{
    while (true)
    {
        std::atomic_thread_fence(std::memory_order_acquire);
        if (this->_threadFinishSpinlock)
        {
            // thread exit signal is caught
            break;
        }

        if (!this->_threadOperationReadStartSpinlock)
        {
            // nothing to do
            continue;
        }

        std::atomic_thread_fence(std::memory_order_acquire);

        size_t readBytes = 0;
        const bool readOk = this->Read(this->_pThreadReadBuffer, this->_threadReadBufferSize, readBytes);

        this->_threadOperationReadStartSpinlock = false;
        this->_threadActuallyReadBytes = readBytes;
        this->_threadReadSucceeded = readOk;
        std::atomic_thread_fence(std::memory_order_release);

        this->_threadOperationReadCompletedSpinlock = true;
        std::atomic_thread_fence(std::memory_order_release);
    }
}

//////////////////////////////////////////////////////////////////////////
