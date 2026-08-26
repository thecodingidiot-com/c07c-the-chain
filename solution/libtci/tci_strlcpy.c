#include "libtci.h"

size_t  tci_strlcpy(char *dst, const char *src, size_t size)
{
    size_t  src_len;
    size_t  i;

    src_len = tci_strlen(src);
    if (size == 0)
        return (src_len);
    /* copy at most size-1 bytes, then null-terminate */
    i = 0;
    while (i < size - 1 && src[i]) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
    /* always returns src_len — even if truncated — so callers can detect truncation */
    return (src_len);
}
