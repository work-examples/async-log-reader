#include "LineReader.h"

#include <assert.h>


namespace
{
    const size_t MaxKnownNtfsClusterSize = 65536;
    const size_t MaxLogLineLength = 1024; // including ending LF/CRLF;

    const size_t ReadChunkSize = MaxKnownNtfsClusterSize * 2;

    // check the formula for ReadBlockSize is correct:
    static_assert(ReadChunkSize >= MaxLogLineLength); // we need this to guarantee there will be no data loss while moving incomplete line to the beginning of the buffer
    static_assert(ReadChunkSize % MaxKnownNtfsClusterSize == 0);

    const size_t ReadBufferSize = ReadChunkSize + MaxLogLineLength;
}


CSyncLineReader::CSyncLineReader()
{
    this->_buffer.Allocate(ReadBufferSize);
}

bool CSyncLineReader::Open(const wchar_t* const filename)
{
    this->Close();
    const bool bAsyncMode = false;
    const bool succeeded = this->_file.Open(filename, bAsyncMode);
    this->_bufferData = std::string_view(this->_buffer.ptr, 0);
    return succeeded;
}

void CSyncLineReader::Close()
{
    this->_file.Close();
}

//__declspec(noinline) // noinline is added to help CPU profiling in release version
std::optional<std::string_view> CSyncLineReader::GetNextLine()
{
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
