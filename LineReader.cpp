#include "LineReader.h"

#include <assert.h>


namespace
{
    const size_t MaxKnownNtfsClusterSize = 65536;
    const size_t MaxLogLineLength = 1024; // including ending LF/CRLF;

    // Max speed was for 256 KB buffer with synchronous file API algorithm; increasing/decreasing its size twice is decreasing the performance
    // For asynchronous file API algorithm (double buffer) I did not found obvious optimal value;
    const size_t ReadChunkSize = MaxKnownNtfsClusterSize * 4;

    // check the formula for ReadBlockSize is correct:
    static_assert(ReadChunkSize >= MaxLogLineLength); // we need this to guarantee there will be no data loss while moving incomplete line to the beginning of the buffer
    static_assert(ReadChunkSize % MaxKnownNtfsClusterSize == 0);

    const size_t ReadBufferSize = ReadChunkSize + MaxLogLineLength;
}


//////////////////////////////////////////////////////////////////////////

CSyncLineReader::CSyncLineReader()
{
    this->_buffer.Allocate(ReadBufferSize);
}

bool CSyncLineReader::Open(const wchar_t* const filename)
{
    if (this->_buffer.ptr == nullptr || filename == nullptr)
    {
        return false;
    }
    this->Close();

    const bool bAsyncMode = false;
    const bool succeeded = this->_file.Open(filename, bAsyncMode);
    if (!succeeded)
    {
        return false;
    }

    this->_bufferData = std::string_view(this->_buffer.ptr, 0);
    return true;
}

void CSyncLineReader::Close()
{
    this->_file.Close();
}

//__declspec(noinline) // noinline is added to help CPU profiling in release version
std::optional<std::string_view> CSyncLineReader::GetNextLine()
{
    if (this->_buffer.ptr == nullptr)
    {
        return {};
    }

    // Find EOL:
    size_t eolOffset = this->_bufferData.find('\n');

    if (eolOffset == this->_bufferData.npos)
    {
        // EOL was not found. Make a choice between last line case and reading additional data from functor.

        if (this->_bufferData.size() > MaxLogLineLength)
        {
            // Incomplete line is already too long
            return {};
        }

        const size_t prefixLength = this->_bufferData.size();
        assert(prefixLength <= MaxLogLineLength && "the rest of buffer is too big for moving to beginning");
        char* const newDataBufferPtr = this->_buffer.ptr + MaxLogLineLength - prefixLength;

        // don't need memmove since the whole high level algorithm will fail if buffers overlap
        memcpy(newDataBufferPtr, this->_bufferData.data(), prefixLength);

        // Read missing data:
        size_t readBytes = 0;
        const bool readOk = this->_file.Read(this->_buffer.ptr + MaxLogLineLength, ReadChunkSize, readBytes);
        if (!readOk)
        {
            // Reading data failed
            return {};
        }

        this->_bufferData = { newDataBufferPtr, prefixLength + readBytes };

        if (this->_bufferData.empty())
        {
            assert(readBytes == 0);
            // Data source already pointed there is not data any more
            // The very last line without LF is not counted
            return {};
        }

        // Search EOL again after reading additional data:
        // I expect we read either ReadChunkSize bytes or we read the data chunk in file.
        eolOffset = this->_bufferData.find('\n', prefixLength);
        if (eolOffset == this->_bufferData.npos)
        {
            if (this->_bufferData.size() > MaxLogLineLength)
            {
                // Incomplete line is too long
                return {};
            }

            // Found last line after reading missing data
            const std::string_view result = this->_bufferData;
            this->_bufferData = { this->_buffer.ptr, 0 };
            assert(!result.empty() && "last line without LF should be not empty");
            return result;
        }
    }

    const size_t foundLineLength = eolOffset + 1;

    if (foundLineLength > MaxLogLineLength)
    {
        // Line is too long
        return {};
    }

    const char* const lineStart = this->_bufferData.data();
    this->_bufferData.remove_prefix(foundLineLength);

    assert(foundLineLength > 0 && "result should contain LF char");
    return std::string_view(lineStart, foundLineLength);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

CAsyncLineReader::CAsyncLineReader()
{
    this->_buffer1.Allocate(ReadBufferSize);
    if (this->_buffer1.ptr != nullptr)
    {
        this->_buffer2.Allocate(ReadBufferSize);
    }
}

bool CAsyncLineReader::Open(const wchar_t* const filename)
{
    if (this->_buffer1.ptr == nullptr || filename == nullptr)
    {
        return false;
    }
    assert(this->_buffer2.ptr != nullptr);
    this->Close();

    const bool bAsyncMode = true;
    const bool succeeded = this->_file.Open(filename, bAsyncMode);
    if (!succeeded)
    {
        return false;
    }

    this->_bufferData = std::string_view(this->_buffer1.ptr, 0);
    this->_firstBufferIsActive = true;

    const bool readStartOk = this->_file.AsyncReadStart(this->_buffer2.ptr + MaxLogLineLength, ReadChunkSize);
    if (!readStartOk)
    {
        this->_file.Close();
        return false;
    }

    return true;
}

void CAsyncLineReader::Close()
{
    this->_file.Close();
}

//__declspec(noinline) // noinline is added to help CPU profiling in release version
std::optional<std::string_view> CAsyncLineReader::GetNextLine()
{
    if (this->_buffer1.ptr == nullptr)
    {
        return {};
    }
    assert(this->_buffer2.ptr != nullptr);

    // Find EOL:
    size_t eolOffset = this->_bufferData.find('\n');

    if (eolOffset == this->_bufferData.npos)
    {
        // EOL was not found. Make a choice between last line case and reading additional data from functor.

        if (this->_bufferData.size() > MaxLogLineLength)
        {
            // Incomplete line is already too long
            return {};
        }

        CCharBuffer& currentBuffer = this->_firstBufferIsActive ? this->_buffer1 : this->_buffer2;
        CCharBuffer& nextBuffer = this->_firstBufferIsActive ? this->_buffer2 : this->_buffer1;

        const size_t prefixLength = this->_bufferData.size();
        assert(prefixLength <= MaxLogLineLength && "the rest of buffer is too big for moving to beginning");
        char* const newDataBufferPtr = nextBuffer.ptr + MaxLogLineLength - prefixLength;

        // don't need memmove since the whole high level algorithm will fail if buffers overlap
        memcpy(newDataBufferPtr, this->_bufferData.data(), prefixLength);

        size_t readBytes = 0;
        const bool readCompleteOk = this->_file.AsyncReadWait(readBytes);
        if (!readCompleteOk)
        {
            // Previous reading failed
            return {};
        }

        // Read missing data:
        const bool readOk = this->_file.AsyncReadStart(currentBuffer.ptr + MaxLogLineLength, ReadChunkSize);
        if (!readOk)
        {
            // New reading failed
            return {};
        }

        this->_bufferData = { newDataBufferPtr, prefixLength + readBytes };
        this->_firstBufferIsActive = !this->_firstBufferIsActive;

        if (this->_bufferData.empty())
        {
            assert(readBytes == 0);
            // Data source already pointed there is not data any more
            // The very last line without LF is not counted
            return {};
        }

        // Search EOL again after reading additional data:
        // I expect we read either ReadChunkSize bytes or we read the data chunk in file.
        eolOffset = this->_bufferData.find('\n', prefixLength);
        if (eolOffset == this->_bufferData.npos)
        {
            if (this->_bufferData.size() > MaxLogLineLength)
            {
                // Incomplete line is too long
                return {};
            }

            // Found last line after reading missing data
            const std::string_view result = this->_bufferData;
            this->_bufferData = { this->_buffer1.ptr, 0 };
            assert(!result.empty() && "last line without LF should be not empty");
            return result;
        }
    }

    const size_t foundLineLength = eolOffset + 1;

    if (foundLineLength > MaxLogLineLength)
    {
        // Line is too long
        return {};
    }

    const char* const lineStart = this->_bufferData.data();
    this->_bufferData.remove_prefix(foundLineLength);

    assert(foundLineLength > 0 && "result should contain LF char");
    return std::string_view(lineStart, foundLineLength);
}
