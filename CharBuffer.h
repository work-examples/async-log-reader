#pragma once

#include <wchar.h> // for size_t


class CCharBuffer
{
public:
    ~CCharBuffer();

    bool Allocate(const size_t bufferLength);
    void Free();

public:
    char* ptr = nullptr;
    size_t size = 0;
};
