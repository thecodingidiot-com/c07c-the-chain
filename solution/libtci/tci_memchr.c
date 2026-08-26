#include "libtci.h"

void    *tci_memchr(const void *s, int c, size_t n)
{
    const unsigned char *p;

    p = (const unsigned char *)s;
    while (n--) {
        if (*p == (unsigned char)c)     /* unsigned char comparison: avoids sign-extension for bytes > 127 */
            return ((void *)p);
        p++;
    }
    return (NULL);
}
