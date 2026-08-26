#include "libtci.h"

size_t  tci_strlen(const char *s)
{
    size_t  len;

    len = 0;
    /* s[len] is zero only at the null terminator; that is the stopping condition */
    while (s[len])
        len++;
    return (len);
}
