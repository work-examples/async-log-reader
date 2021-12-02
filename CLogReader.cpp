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

    // TODO: implement!!!!!!!!!
    return false;
}
