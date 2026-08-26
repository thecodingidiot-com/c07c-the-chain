#include "libtci.h"

char    *tci_strncpy(char *dst, const char *src, size_t n)
{
    size_t  i;

    i = 0;
    while (i < n && src[i]) {
        dst[i] = src[i];
        i++;
    }
    /* C spec: pad remaining bytes with '\0' even past the end of src */
    while (i < n)
        dst[i++] = '\0';
    return (dst);
}
