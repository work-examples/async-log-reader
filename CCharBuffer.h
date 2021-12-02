#pragma once

#include <stddef.h>


class CCharBuffer
{
public:
    ~CCharBuffer();

    void Allocate(const size_t bufferLength);
    void Free();

public:
    char* ptr = nullptr;
    size_t length = 0;
};
