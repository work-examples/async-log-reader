#include "TestHelpers.h"

#include <fstream>

#include <atlcomcli.h>

#include <windows.h>

#include "gtest/gtest.h"


TempFile::TempFile(const std::string& data)
{
    const std::string tempDir = testing::TempDir();
    WCHAR filename[MAX_PATH] = L"";
    const bool ok = !!GetTempFileNameW(CA2W(tempDir.c_str()), L"LLR", 0, filename);
    if (!ok)
    {
        throw std::exception("GetTempFileNameW failed");
    }
    this->_filename = filename;

    // write file
    std::ofstream file(this->_filename.c_str(), std::ios::binary);
    file << data;
    file.close();
}

TempFile::~TempFile()
{
    _wunlink(this->_filename.c_str());
}
