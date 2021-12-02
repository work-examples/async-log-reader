#include "CLineReader.h"

#include <assert.h>


namespace
{
    const size_t MaxKnownNtfsClusterSize = 65536;
    const size_t MaxLogLineLength = 1024; // including ending LF/CRLF

    // We need to read lots of lines in a single file read operation to optimize speed:
    // - less jumps to kernel mode
    // - less memory copying of the last partially read line
    const size_t MinimumLinesInReadBlock = 100;
    const size_t ReadChunkSize = (MaxLogLineLength * MinimumLinesInReadBlock + MaxKnownNtfsClusterSize - 1) / MaxKnownNtfsClusterSize * MaxKnownNtfsClusterSize;

    // check the formula for ReadBlockSize is correct:
    static_assert(ReadChunkSize >= MaxLogLineLength * MinimumLinesInReadBlock);
    static_assert(ReadChunkSize < MaxLogLineLength* MinimumLinesInReadBlock + MaxKnownNtfsClusterSize);
    static_assert(ReadChunkSize % MaxKnownNtfsClusterSize == 0);

    const size_t ReadBufferSize = ReadChunkSize + MaxLogLineLength - 1;
}


CLineReader::CLineReader()
{
    this->_buffer.Allocate(ReadBufferSize);
}

bool CLineReader::Setup(std::function<CLineReader::ReadDataFunc> readData)
{
    if (!readData || this->_buffer.ptr == nullptr)
    {
        // Bad argument or failed to allocate memory
        return false;
    }

    this->_funcReadData = std::move(readData);

    // Prefill buffer to satisfy its invariant
    size_t readBytes = 0;
    const bool readOk = this->_funcReadData(this->_buffer.ptr, ReadChunkSize, readBytes);
    if (!readOk)
    {
        // Failed to prefill data buffer
        return false;
    }

    this->_bufferData = { this->_buffer.ptr, readBytes };
    return true;
}

std::optional<std::string_view> CLineReader::GetNextLine()
{
    if (this->_bufferData.empty())
    {
        BLIAD!!!!
        // End of file was found previously
        return {};
    }

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

        // During the last chunk of data _bufferData will be smaller than ReadChunkSize:
        // /====================\
        // |      _buffer       |
        // | ReadChunkSize |    |
        // |  |_bufferData|     |
        // \====================/
        const bool lastChunk = this->_bufferData.data() + this->_bufferData.size() < this->_buffer.ptr + ReadChunkSize;
        if (lastChunk)
        {
            // Last line
            std::string_view result;
            this->_bufferData.swap(result);
            return result;
        }

        if (this->_bufferData.size() == MaxLogLineLength)
        {
            // Incomplete line cannot be moved and concatenated later because of its critical length
            return {};
        }

        const size_t prefixLength = this->_bufferData.size();
        assert(prefixLength < MaxLogLineLength);

        memmove(this->_buffer.ptr, this->_bufferData.data(), prefixLength);

        // Read missing data:
        if (!this->_funcReadData)
        {
            // _readData functor is not initialized by Setup()
            return {};
        }

        size_t readBytes = 0;
        assert(prefixLength + ReadChunkSize <= ReadBufferSize);
        const bool readOk = this->_funcReadData(this->_buffer.ptr + prefixLength, ReadChunkSize, readBytes);
        if (!readOk)
        {
            // Reading data failed
            return {};
        }

        this->_bufferData = { this->_buffer.ptr, prefixLength + ReadChunkSize };

        // Search EOL again:
        eolOffset = this->_bufferData.find(chEOL, prefixLength);
        if (eolOffset == this->_bufferData.npos)
        {
            if (this->_bufferData.size() > MaxLogLineLength)
            {
                // Incomplete line is too long
                return {};
            }

            // Found last line after reading missing data
            std::string_view result;
            this->_bufferData.swap(result);
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

    return std::string_view(lineStart, foundLineLength);
}
