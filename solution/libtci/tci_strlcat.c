#include "libtci.h"

size_t  tci_strlcat(char *dst, const char *src, size_t size)
{
    size_t  dst_len;
    size_t  src_len;
    size_t  i;

    dst_len = tci_strlen(dst);
    src_len = tci_strlen(src);
    /* if dst is already at or beyond size, no room to append */
    if (size <= dst_len)
        return (size + src_len);
    i = 0;
    while (src[i] && dst_len + i < size - 1) {
        dst[dst_len + i] = src[i];
        i++;
    }
    dst[dst_len + i] = '\0';
    /* return dst_len + src_len regardless of truncation: callers compare against size to detect it */
    return (dst_len + src_len);
}
