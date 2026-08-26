#include "libtci.h"

int     tci_isalnum(int c)
{
    /* delegates to the two predicates: any letter or decimal digit */
    return (tci_isalpha(c) || tci_isdigit(c));
}
