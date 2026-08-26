#include "libtciutil.h"

static int  count_fields(char const *s, char sep)
{
    int     count;
    int     in_field;

    count = 0;
    in_field = 0;
    while (*s) {
        if (*s != sep && !in_field) {
            count++;            /* first non-sep character opens a new field */
            in_field = 1;
        } else if (*s == sep) {
            in_field = 0;       /* separator closes the current field */
        }
        s++;
    }
    return (count);
}

static char **fill_words(char const *s, char sep, int count)
{
    char    **words;
    int     i;
    int     len;

    words = tci_calloc(count + 1, sizeof(char *)); /* +1 for NULL terminator */
    if (!words)
        return (NULL);
    i = 0;
    while (*s) {
        if (*s != sep) {
            len = 0;
            while (s[len] && s[len] != sep)       /* measure the field */
                len++;
            words[i++] = tci_strndup(s, len);      /* copy exactly len chars */
            s += len;
        } else {
            s++;                                   /* skip separator */
        }
    }
    words[i] = NULL;                               /* NULL-terminate the array */
    return (words);
}

char    **tciu_split(char const *s, char sep)
{
    int  count;

    if (!s)                              /* NULL input: nothing to split */
        return (NULL);
    count = count_fields(s, sep);
    return (fill_words(s, sep, count));  /* caller owns the returned array */
}
