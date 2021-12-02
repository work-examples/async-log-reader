#include "CLogReader.h"

namespace
{
    const size_t MaxKnownNtfsClusterSize = 65536;
    const size_t MaxLogLineLength = 1024;

    // We need to read lots of lines in a single file read operation to optimize speed:
    // - less jumps to kernel mode
    // - less memory copying of the last partially read line
    const size_t MinimumLinesInReadBlock = 100;
    const size_t ReadBlockSize = (MaxLogLineLength * MinimumLinesInReadBlock + MaxKnownNtfsClusterSize - 1) / MaxKnownNtfsClusterSize * MaxKnownNtfsClusterSize;

    // check the formula for ReadBlockSize is correct:
    static_assert(ReadBlockSize >= MaxLogLineLength * MinimumLinesInReadBlock);
    static_assert(ReadBlockSize < MaxLogLineLength* MinimumLinesInReadBlock + MaxKnownNtfsClusterSize);
    static_assert(ReadBlockSize% MaxKnownNtfsClusterSize == 0);

    const size_t ReadBufferSize = ReadBlockSize + MaxLogLineLength;
}


CLogReader::CLogReader(): _lineMatcher(MaxLogLineLength)
{
    this->_buffer.Allocate(ReadBufferSize);
}

bool CLogReader::Open(const wchar_t* const filename)
{
    if (this->_buffer.ptr == nullptr)
    {
        // Could not allocate memory in constructor for read buffer
        return false;
    }

    CLogReader::Close();

    const bool succeeded = this->_file.Open(filename);

    this->_bufferBeginOffset = 0;
    this->_bufferEndOffset = 0;

    if (succeeded)
    {
        size_t readBytes = 0;
        const bool readOk = this->_file.Read(this->_buffer.ptr, ReadBlockSize, this->_bufferEndOffset);
        if (!readOk)
        {
            // Failed to pre-fill data buffer
            return false;
        }
    }

    return succeeded;
}

void CLogReader::Close()
{
    this->_file.Close();
}

bool CLogReader::SetFilter(const char* const filter)
{
    const bool succeeded = this->_lineMatcher.SetFilter(filter);
    return succeeded;
}

bool CLogReader::GetNextLine(char* buf, const size_t bufsize, size_t& readBytes)
{
    if (buf == nullptr)
    {
        return false;
    }

    {
        if (this->_bufferEndOffset == 0)
        {
            // End of file was encountered previously
            return false;
        }

        const char* const dataBegin = this->_buffer.ptr + this->_bufferBeginOffset;
        const size_t dataLength = this->_bufferEndOffset - this->_bufferBeginOffset;

        // Find EOL:
        const char chEOL = '\n';
        const char* const eol = static_cast<const char*>(memchr(dataBegin, chEOL, dataLength));

        // TODO: what to do if EOL was not found during MaxLineLen or it was found but later????????????????????????

        if (eol == nullptr && this->_bufferEndOffset >= ReadBlockSize)
        {
            // TODO: move memory to begin of buffer + readfile + try again to match starting from dataLength offset
        }

        const size_t lineLength = eol == nullptr ? dataLength : eol + 1 - dataBegin;

        // Copy found line to provided buffer:
        if (bufsize < lineLength)
        {
            // User buffer size is too small
            return false;
        }

        // TODO: check if match and loop???????

        memcpy(buf, this->_buffer.ptr + this->_bufferBeginOffset, lineLength);
        readBytes = lineLength;
        this->_bufferBeginOffset += lineLength;
    }

    return true;
}
