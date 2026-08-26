#include "libtci.h"

int     tci_strcmp(const char *s1, const char *s2)
{
    while (*s1 && *s1 == *s2) {
        s1++;
        s2++;
    }
    /* unsigned char subtraction gives the correct signed result for all byte values */
    return ((unsigned char)*s1 - (unsigned char)*s2);
}
