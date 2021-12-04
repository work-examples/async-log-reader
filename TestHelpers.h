#pragma once

#include <string>


class TempFile
{
public:
    TempFile(const std::string& data);
    ~TempFile();

    const std::wstring& GetFilename() const
    {
        return this->_filename;
    }

protected:
    std::wstring _filename;
};
