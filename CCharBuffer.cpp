#include "CCharBuffer.h"

#include <stdlib.h>


CCharBuffer::~CCharBuffer()
{
    Free();
}

bool CCharBuffer::Allocate(const size_t bufferLength)
{
    Free();

    if (bufferLength == 0)
    {
        ptr = static_cast<char*>(malloc(1));
        size = 0;
    }
    else
    {
        ptr = static_cast<char*>(malloc(bufferLength));
        size = ptr != nullptr ? bufferLength : 0;
    }

    return ptr != nullptr;
}

void CCharBuffer::Free()
{
    if (ptr)
    {
        free(ptr);
        ptr = nullptr;
        size = 0;
    }
}
