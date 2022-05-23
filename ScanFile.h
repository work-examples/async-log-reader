#pragma once

#include <atomic>      // this is STL, but it does not need exceptions
#include <mutex>       // this is STL, but it does not need exceptions
#include <new> // for std::hardware_constructive_interference_size
#include <optional>    // this is STL, but it does not need exceptions
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

    // Current limitation: only one async operation can be in progress.
    bool LockFreecInit();
    void LockFreecClean();
    bool LockFreeReadStart(char* const buffer, const size_t bufferLength);
    bool LockFreeReadWait(size_t& readBytes);
    void LockFreeThreadProc();

protected:
    HANDLE              _hFile           = nullptr;

    // For memory mapping:
    HANDLE              _hFileMapping    = nullptr;
    void*               _pViewOfFile     = nullptr;

    // For async IO:
    HANDLE              _hAsyncEvent     = nullptr;
    LARGE_INTEGER       _asyncFileOffset = {};
    OVERLAPPED          _asyncOverlapped = {};
    bool                _asyncOperationInProgress = false;

    // Separate thread + Lock free:
    HANDLE              _hThread         = nullptr;
    bool                _threadOperationInProgress = false; // for protection against wrong API usage, not for use in a worker thread, no memory protection
    // Spin lock variables:
    // We could use kernel-mode events instead of spin locks, it will save CPU, but will loose time during synchronization (switch to kernel).
    alignas(std::hardware_constructive_interference_size) // small speedup to get a bunch of variables into single cache line
    bool                _threadFinishSpinlock = false;
    bool                _threadOperationReadStartSpinlock = false;
    bool                _threadOperationReadCompletedSpinlock = false;
    // other data protected by spin locks:
    char*               _pThreadReadBuffer       = nullptr;
    size_t              _threadReadBufferSize    = 0;
    size_t              _threadActuallyReadBytes = 0;
    bool                _threadReadSucceeded     = false;
};
