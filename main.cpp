#include <fcntl.h>
#include <io.h>
#include <stdio.h>

#include <atlcomcli.h>

#include "LogReader.h"


int wmain(const int argc, const wchar_t* const argv[])
{
    if (argc <= 2)
    {
        fwprintf(stderr, L"Error! Not enough command line arguments!\n");
        fwprintf(stderr, L"Usage:\n");
        fwprintf(stderr, L"LogReader.exe <filename> <pattern>\n");
        fwprintf(stderr, L"Pattern is similar to fnmatch and supports symbols '*' and '?'.\n");
        fwprintf(stderr, L"Example:\n");
        fwprintf(stderr, L"LogReader.exe 20190102.log \"*bbb*\"\n");
        return 1;
    }

    CLogReader reader;

    const wchar_t* const fileName = argv[1];
    const wchar_t* const lineFilter = argv[2];

    const bool openedOk = reader.Open(fileName);
    if (!openedOk)
    {
        fwprintf(stderr, L"Error! Failed to open file: \"%ws\"\n", fileName);
        return 2;
    }

    const bool filterSetOk = reader.SetFilter(CW2A(lineFilter));
    if (!filterSetOk)
    {
        fwprintf(stderr, L"Error! Failed to set filter: \"%ws\"\n", lineFilter);
        return 3;
    }

    // prevent printf from changing LF to CRLF
    // so we act the same way as grep does
    _setmode(_fileno(stdout), O_BINARY);

    while (true)
    {
        const auto line = reader.GetNextLine();
        if (!line)
        {
            break;
        }

        fwrite(line->data(), line->size(), 1, stdout);

        // Add optionally missing EOL after the last line of the file:
        // CLineReader gives guarantee line is never empty, but it is better to check to be safe:
        //if (line->size() > 0)
        //{
        //    if (line->data()[line->size() - 1] != '\n')
        //    {
        //        fwrite("\n", 1, 1, stdout);
        //    }
        //}
    }

    reader.Close();

    return 0;
}
