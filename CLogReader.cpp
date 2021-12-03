#include "CLogReader.h"


CLogReader::CLogReader(): _lineMatcher(CLineReader::g_MaxLogLineLength)
{
}

bool CLogReader::Open(const wchar_t* const filename)
{
    this->Close();

    const bool succeeded = this->_file.Open(filename);
    if (succeeded)
    {
        std::function<CLineReader::ReadDataFunc> funcReadData =
            std::bind(&CScanFile::Read, this->_file, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

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
    const bool succeeded = this->_lineMatcher.SetFilter(filter);
    return succeeded;
}

std::optional<std::string_view> CLogReader::GetNextLine()
{
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

        const bool matched = this->_lineMatcher.CheckMatch(matchView);
        if (matched)
        {
            // line matched
            return line;
        }
    }
}
