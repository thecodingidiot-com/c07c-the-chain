#include "libtci.h"

int     tci_isprint(int c)
{
    /* 32 (space) through 126 (~): every visible glyph; 127 is DEL */
    return (c >= 32 && c <= 126);
}
