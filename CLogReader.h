#pragma once

#include "CCharBuffer.h"
#include "CFnMatch.h"
#include "CLineReader.h"
#include "CScanFile.h"

#include <optional>
#include <string_view>

#include <wchar.h> // for size_t, wchar_t


class CLogReader final
{
public:
    CLogReader();

    // open file; return false on error
    bool Open(const wchar_t* const filename);
    // close file
    void Close();

    // set line filter; return false on error
    bool SetFilter(const char* const filter);

    // request next matching line; line may contain '\0' and may end with '\n'; return false on error or EOF
    std::optional<std::string_view> GetNextLine();

    // request next matching line; line may contain '\0' and may end with '\n'; return false on error or EOF
    // NOTE: I would prefer to drop this method because it needs additional memory copy and do not support null characters inside of the line.
    //       Keeping the method for compatibility with original requirements.
    bool GetNextLine(char* buf, const size_t bufsize)
    {
        if (buf == nullptr)
        {
            return false;
        }
        const auto line = this->GetNextLine();
        if (!line || line->size() >= bufsize)
        {
            return false;
        }
        memcpy(buf, line->data(), line->size());
        buf[line->size()] = '\0';
        return true;
    }

protected:
    CScanFile   _file;
    CLineReader _lineReader;
    CFnMatch    _lineMatcher;
};
