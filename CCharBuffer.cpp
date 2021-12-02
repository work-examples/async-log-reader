#include "CCharBuffer.h"

#include <stdlib.h>


CCharBuffer::~CCharBuffer()
{
    this->Free();
}

void CCharBuffer::Allocate(const size_t bufferLength)
{
    this->Free();

    if (bufferLength)
    {
        this->ptr = static_cast<char*>(malloc(bufferLength));
        this->length = bufferLength;
    }
}

void CCharBuffer::Free()
{
    if (this->ptr)
    {
        free(this->ptr);
        this->ptr = nullptr;
        this->length = 0;
    }
}
