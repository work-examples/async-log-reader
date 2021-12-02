#include "CCharBuffer.h"

#include <stdlib.h>


CCharBuffer::~CCharBuffer()
{
    this->Free();
}

bool CCharBuffer::Allocate(const size_t bufferLength)
{
    this->Free();

    if (bufferLength == 0)
    {
        this->ptr = static_cast<char*>(malloc(1));
        this->size = 0;
    }
    else
    {
        this->ptr = static_cast<char*>(malloc(bufferLength));
        size = this->ptr != nullptr ? bufferLength : 0;
    }

    return this->ptr != nullptr;
}

void CCharBuffer::Free()
{
    if (this->ptr)
    {
        free(this->ptr);
        this->ptr = nullptr;
        this->size = 0;
    }
}
