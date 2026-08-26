#include "libtci.h"

int     tci_isspace(int c)
{
    /* the six C whitespace bytes: space, tab, newline, vertical-tab, form-feed, carriage-return */
    return (c == ' ' || c == '\t' || c == '\n'
        || c == '\v' || c == '\f' || c == '\r');
}
