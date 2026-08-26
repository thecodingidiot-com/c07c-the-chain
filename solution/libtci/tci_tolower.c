#include "libtci.h"

int     tci_tolower(int c)
{
    if (c >= 'A' && c <= 'Z')
        return (c + 32);    /* 'A' is 65, 'a' is 97: adding 32 converts uppercase to lowercase */
    return (c);
}
