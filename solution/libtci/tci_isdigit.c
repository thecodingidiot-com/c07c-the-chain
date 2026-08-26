#include "libtci.h"

int     tci_isdigit(int c)
{
    /* '0'–'9' only; not locale-dependent */
    return (c >= '0' && c <= '9');
}
