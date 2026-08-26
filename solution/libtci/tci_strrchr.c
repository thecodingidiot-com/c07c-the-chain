#include "libtci.h"

char    *tci_strrchr(const char *s, int c)
{
    const char  *last;

    last = NULL;
    while (*s) {
        if (*s == (char)c)
            last = s;   /* last tracks the most recent match; loop continues to find further ones */
        s++;
    }
    /* check the null terminator itself */
    if ((char)c == '\0')
        return ((char *)s);
    return ((char *)last);
}
