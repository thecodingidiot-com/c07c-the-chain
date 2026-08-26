#include "libtci.h"

int     tci_strncmp(const char *s1, const char *s2, size_t n)
{
    /* n=0: strings compare equal by definition (no bytes examined) */
    if (n == 0)
        return (0);
    while (n > 1 && *s1 && *s1 == *s2) {
        s1++;
        s2++;
        n--;
    }
    return ((unsigned char)*s1 - (unsigned char)*s2);
}
