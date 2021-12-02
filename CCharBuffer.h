#pragma once

#include <stddef.h>


class CCharBuffer
{
public:
    ~CCharBuffer();

    bool Allocate(const size_t bufferLength);
    void Free();

public:
    char* ptr = nullptr;
    size_t length = 0;
};
