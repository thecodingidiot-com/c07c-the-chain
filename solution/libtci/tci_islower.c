#include "libtci.h"

int     tci_islower(int c)
{
    /* 'a'–'z' in ASCII are contiguous: 97–122 */
    return (c >= 'a' && c <= 'z');
}
