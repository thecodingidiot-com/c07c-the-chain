#include "libtci.h"

char    *tci_strchr(const char *s, int c)
{
    while (*s) {
        if (*s == (char)c)
            return ((char *)s);
        s++;
    }
    /* check the null terminator itself: strchr(s, '\0') must return a pointer to it */
    if ((char)c == '\0')
        return ((char *)s);
    return (NULL);
}
