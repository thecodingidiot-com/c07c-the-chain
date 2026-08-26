#include "libtci.h"

int     tci_isupper(int c)
{
    /* 'A'–'Z' in ASCII are contiguous: 65–90 */
    return (c >= 'A' && c <= 'Z');
}
