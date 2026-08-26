#include "libtci.h"
#include <stdlib.h>

char    *tci_strndup(const char *s, size_t n)
{
    char    *copy;
    size_t  i;

    copy = tci_calloc(n + 1, 1);      /* +1 for null terminator */
    if (!copy)
        return (NULL);
    i = 0;
    while (i < n && s[i]) {          /* stop at n or at '\0', whichever comes first */
        copy[i] = s[i];
        i++;
    }
    return (copy);                   /* caller owns this allocation */
}
