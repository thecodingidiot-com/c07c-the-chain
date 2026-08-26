#include "libtci.h"

void    *tci_memcpy(void *dst, const void *src, size_t n)
{
    unsigned char       *d;
    const unsigned char *s;

    /* cast to unsigned char: copying through void * is defined only for unsigned char */
    d = (unsigned char *)dst;
    s = (const unsigned char *)src;
    while (n--)
        *d++ = *s++;
    return (dst);
}
