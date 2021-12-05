#pragma once

#include <atomic> // this is STL, but it does not need exceptions
#include <mutex> // this is STL, but it does not need exceptions
#include <optional> // this is STL, but it does not need exceptions
#include <string_view> // this is STL, but it does not need exceptions
#include <thread> // this is STL, but it does not need exceptions

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

protected:
    void LockFreeThread();

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

    // Separate thread + Lock free:
    std::unique_ptr<std::thread> _pThread = nullptr;
    bool          _threadOperationInProgress = false; // for protection against wrong API usage
    // Spinlock variabled:
    // We could use kernel-mode events instead of spinlocks, it will save CPU, but will loose time during synchronization (switch to kernel).
    std::atomic<bool> _threadFinishSignal = ATOMIC_VAR_INIT(false);
    std::atomic<bool> _threadOperationReadStartSignal = ATOMIC_VAR_INIT(false);
    std::atomic<bool> _threadOperationReadCompletedSignal = ATOMIC_VAR_INIT(false);
    // We do not actually need std::mutex here. We could use just memory fence instead.
    // But current solution would be easier for code review and it will be more error-safe when using std::mutex.
    // It will never wait in kernel mode since we will try to lock mutex only when it is guaranteed to be free.
    std::mutex    _protectThreadData; // this is CRITICAL_SECTION in Windows to protect and sync the data below:
    char*         _pThreadReadBuffer = nullptr;
    size_t        _threadReadBufferSize = 0;
    size_t        _threadActuallyReadBytes = 0;
    bool          _threadReadSucceeded = false;
};
