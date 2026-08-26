#include "libtci.h"
#include <unistd.h>  /* read */
#include <stdlib.h>  /* free */

#ifndef BUFFER_SIZE
# define BUFFER_SIZE 32  /* fallback if not set via -D */
#endif

# define FD_MAX 1024

static char     *leftover[FD_MAX];  /* one slot per file descriptor */

static char     *strjoin(const char *s1, const char *s2)
{
    char    *result;
    size_t  len;         /* combined length of both strings */
    size_t  i;
    size_t  j;

    if (!s1 || !s2)
        return (NULL);
    len = tci_strlen(s1) + tci_strlen(s2);
    result = tci_calloc(len + 1, 1);  /* +1 for null terminator */
    if (!result)
        return (NULL);
    i = 0;
    while (s1[i]) {
        result[i] = s1[i];
        i++;
    }
    j = 0;
    while (s2[j]) {                  /* append s2 after s1 */
        result[i + j] = s2[j];
        j++;
    }
    return (result);                 /* caller owns this allocation */
}

static char     *extract_line(char **lo, char *nl)
{
    char    *line;
    char    *remainder;

    line = tci_strndup(*lo, nl - *lo + 1); /* up to and including '\n' */
    remainder = tci_strdup(nl + 1);        /* everything after the '\n' */
    free(*lo);                            /* old leftover is replaced */
    *lo = remainder;
    return (line);                        /* caller owns this allocation */
}

static char     *flush_leftover(char **lo)
{
    char    *last;

    if (!*lo || !**lo) {  /* NULL or empty string — nothing left */
        free(*lo);
        *lo = NULL;
        return (NULL);
    }
    last = *lo;           /* last line has no trailing '\n' */
    *lo = NULL;           /* next call starts clean */
    return (last);        /* caller owns this allocation */
}

char    *tci_getline(int fd)
{
    char    buf[BUFFER_SIZE + 1];   /* +1 for null terminator */
    char    *nl;
    char    *tmp;
    int     bytes;

    if (fd < 0 || fd >= FD_MAX)     /* reject out-of-range descriptors */
        return (NULL);
    if (!leftover[fd])
        leftover[fd] = tci_strdup(""); /* initialise slot on first call for this fd */
    while (1) {
        nl = tci_strchr(leftover[fd], '\n');
        if (nl)
            return (extract_line(&leftover[fd], nl)); /* line ready — no read needed */
        bytes = read(fd, buf, BUFFER_SIZE);
        if (bytes <= 0)
            return (flush_leftover(&leftover[fd]));   /* EOF or error */
        buf[bytes] = '\0';          /* read() does not null-terminate */
        tmp = leftover[fd];
        leftover[fd] = strjoin(leftover[fd], buf);    /* accumulate new chunk */
        free(tmp);                  /* free old leftover after joining */
    }
}
