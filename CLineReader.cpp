#include "CLineReader.h"

#include <assert.h>


namespace
{
    const size_t MaxKnownNtfsClusterSize = 65536;
    const size_t MaxLogLineLength = CLineReader::g_MaxLogLineLength; // including ending LF/CRLF;

    // We need to read lots of lines in a single file read operation to optimize speed:
    // - less jumps to kernel mode
    // - less memory copying of the last partially read line
    const size_t MinimumLinesInReadBlock = 100;
    const size_t ReadChunkSize = (MaxLogLineLength * MinimumLinesInReadBlock + MaxKnownNtfsClusterSize - 1) / MaxKnownNtfsClusterSize * MaxKnownNtfsClusterSize;

    // check the formula for ReadBlockSize is correct:
    static_assert(ReadChunkSize >= MaxLogLineLength * 2);
    static_assert(ReadChunkSize >= MaxLogLineLength * MinimumLinesInReadBlock);
    static_assert(ReadChunkSize < MaxLogLineLength* MinimumLinesInReadBlock + MaxKnownNtfsClusterSize);
    static_assert(ReadChunkSize % MaxKnownNtfsClusterSize == 0);

    const size_t ReadBufferSize = ReadChunkSize + MaxLogLineLength;
}


CLineReader::CLineReader()
{
    this->_buffer.Allocate(ReadBufferSize);
}

bool CLineReader::Setup(const std::function<CLineReader::ReadDataFunc>& readData)
{
    if (!readData || this->_buffer.ptr == nullptr)
    {
        // Bad argument or failed to allocate memory
        return false;
    }

    this->_funcReadData = readData;
    this->_bufferData = { this->_buffer.ptr, 0 };
    return true;
}

//__declspec(noinline) // noinline is added to help CPU profiling in release version
std::optional<std::string_view> CLineReader::GetNextLine()
{
    // Find EOL:
    const char chEOL = '\n';
    size_t eolOffset = this->_bufferData.find(chEOL);

    if (eolOffset == this->_bufferData.npos)
    {
        // EOL was not found. Make a choice between last line case and reading additional data from functor.

        if (this->_bufferData.size() > MaxLogLineLength)
        {
            // Incomplete line is already too long
            return {};
        }

        const size_t prefixLength = this->_bufferData.size();
        assert(prefixLength <= MaxLogLineLength && "the rest of buffer is not too big for moving to beginning");

        memmove(this->_buffer.ptr, this->_bufferData.data(), prefixLength);

        // Read missing data:
        if (!this->_funcReadData)
        {
            // _readData functor is not initialized by Setup()
            return {};
        }

        size_t readBytes = 0;
        assert(prefixLength + ReadChunkSize <= ReadBufferSize && "buffer is big enough to receive all data");
        const bool readOk = this->_funcReadData(this->_buffer.ptr + prefixLength, ReadChunkSize, readBytes);
        if (!readOk)
        {
            // Reading data failed
            return {};
        }

        this->_bufferData = { this->_buffer.ptr, prefixLength + readBytes };

        if (this->_bufferData.empty())
        {
            assert(readBytes == 0);
            // Data source already pointed there is not data any more
            // The very last line without LF is not counted
            return {};
        }

        // Search EOL again after reading additional data:
        // I expect we read either ReadChunkSize bytes or we read the data chunk in file.
        eolOffset = this->_bufferData.find(chEOL, prefixLength);
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
            assert(!result.empty() && "last line without LF should be non empty");
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
