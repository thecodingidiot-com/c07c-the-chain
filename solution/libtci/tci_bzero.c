#include "libtci.h"

void    tci_bzero(void *s, size_t n)
{
    unsigned char   *p;

    /* equivalent to memset(s, 0, n): zero every byte without reading src */
    p = (unsigned char *)s;
    while (n--)
        *p++ = 0;
}
