#include "libtci.h"

void    *tci_memset(void *s, int c, size_t n)
{
    unsigned char   *p;

    /* cast to unsigned char: writing int through void * is defined only for unsigned char */
    p = (unsigned char *)s;
    while (n--)
        *p++ = (unsigned char)c;
    return (s);
}
