#include "libtci.h"

char    *tci_strnstr(const char *haystack, const char *needle, size_t len)
{
    size_t  nlen;
    size_t  i;
    size_t  j;

    if (!*needle)
        return ((char *)haystack);
    nlen = tci_strlen(needle);
    i = 0;
    while (i < len && haystack[i]) {
        /* needle must fit entirely within the remaining len bytes */
        if (i + nlen > len)
            break;
        j = 0;
        while (j < nlen && haystack[i + j] == needle[j])
            j++;
        if (j == nlen)
            return ((char *)haystack + i);
        i++;
    }
    return (NULL);
}
