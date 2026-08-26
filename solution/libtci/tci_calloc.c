#include "libtci.h"
#include <stdint.h>

void    *tci_calloc(size_t count, size_t size)
{
    void    *p;

    /* guard against overflow before multiplying */
    if (count != 0 && size > SIZE_MAX / count)
        return (NULL);
    p = malloc(count * size);
    if (!p)
        return (NULL);
    tci_bzero(p, count * size);  /* tci_bzero zeroes count*size bytes: calloc guarantees zero-initialisation */
    return (p);
}
