#include "CLogReader.h"


bool CLogReader::Open(const wchar_t* const filename)
{
    this->Close();

    const bool succeeded = this->_file.Open(filename);
    if (succeeded)
    {
        const std::function<CLineReader::ReadDataFunc> funcReadData =
            std::bind(&CScanFile::Read, &this->_file, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

        const bool setupOk = this->_lineReader.Setup(funcReadData);
        if (!setupOk)
        {
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
    if (filter == nullptr)
    {
        return false;
    }

    const size_t patternLen = strlen(filter);
    const bool allocatedOk = this->_pattern.Allocate(patternLen + 1);
    if (!allocatedOk)
    {
        return false;
    }

    memcpy(this->_pattern.ptr, filter, patternLen + 1);
    this->_pattern.size -= 1; // ignore terminating zero

    return true;
}

std::optional<std::string_view> CLogReader::GetNextLine()
{
    const std::string_view pattern = { this->_pattern.ptr, this->_pattern.size };

    while (true)
    {
        const auto line = this->_lineReader.GetNextLine();
        if (!line)
        {
            // error or end of file
            return {};
        }

        std::string_view matchView = *line;

        // Ignore CRLF/LF during matching:
        if (!matchView.empty() && matchView.back() == '\n')
        {
            matchView.remove_suffix(1);
            if (!matchView.empty() && matchView.back() == '\r')
            {
                matchView.remove_suffix(1);
            }
        }

        const bool matched = this->_lineMatcher.Match(matchView, pattern);
        if (matched)
        {
            // line matched
            return line;
        }
    }
}
