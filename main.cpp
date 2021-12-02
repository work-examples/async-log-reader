#include <stdio.h>

#include <atlcomcli.h>

#include "CLogReader.h"


int wmain(const int argc, const wchar_t* const argv[])
{
    if (argc <= 2)
    {
        wprintf(L"Error! Not enough command line arguments!\n");
        wprintf(L"Usage:\n");
        wprintf(L"LogReader.exe <filename> <pattern>\n");
        wprintf(L"Pattern is similar to fnmatch and supports symbols '*' and '?'.\n");
        wprintf(L"Example:\n");
        wprintf(L"LogReader.exe 20190102.log \"*bbb*\"\n");
        return 1;
    }

    CLogReader reader;

    const wchar_t* const fileName = argv[1];
    const wchar_t* const lineFilter = argv[2];

    const bool openedOk = reader.Open(fileName);
    if (!openedOk)
    {
        wprintf(L"Error! Failed to open file: \"%ws\"\n", fileName);
        return 2;
    }

    const bool filterSetOk = reader.SetFilter(CW2A(lineFilter));
    if (!filterSetOk)
    {
        wprintf(L"Error! Failed to set filter: \"%ws\"\n", lineFilter);
        return 2;
    }

    char buffer[2000] = "";

    while (true)
    {
        size_t readBytes = 0;
        const bool readOk = reader.GetNextLine(buffer, sizeof(buffer), readBytes);
        if (!readOk)
        {
            break;
        }

        if (readBytes > 0)
        {
            fwrite(buffer, readBytes, 1, stdout);

            // Add missing EOL for the last line of file:
            if (buffer[readBytes - 1] != '\n')
            {
                fwrite("\n", 1, 1, stdout);
            }
        }
    }

    reader.Close();

    return 0;
}
