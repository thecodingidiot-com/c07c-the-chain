#ifndef LIBTCI_H
# define LIBTCI_H

# include <stddef.h>
# include <stdlib.h>

/* memory */
void    *tci_memset(void *s, int c, size_t n);
void    *tci_memcpy(void *dst, const void *src, size_t n);
void    *tci_memmove(void *dst, const void *src, size_t n);
void    *tci_memchr(const void *s, int c, size_t n);
void     tci_bzero(void *s, size_t n);
void    *tci_calloc(size_t count, size_t size);

/* character classification */
int      tci_isascii(int c);
int      tci_isalpha(int c);
int      tci_isdigit(int c);
int      tci_isalnum(int c);
int      tci_isspace(int c);
int      tci_isupper(int c);
int      tci_islower(int c);
int      tci_isprint(int c);
int      tci_toupper(int c);
int      tci_tolower(int c);

/* strings */
size_t   tci_strlen(const char *s);
char    *tci_strcpy(char *dst, const char *src);
char    *tci_strncpy(char *dst, const char *src, size_t n);
size_t   tci_strlcpy(char *dst, const char *src, size_t size);
size_t   tci_strlcat(char *dst, const char *src, size_t size);
int      tci_strcmp(const char *s1, const char *s2);
int      tci_strncmp(const char *s1, const char *s2, size_t n);
char    *tci_strchr(const char *s, int c);
char    *tci_strrchr(const char *s, int c);
char    *tci_strdup(const char *s);
char    *tci_strndup(const char *s, size_t n);
char    *tci_strnstr(const char *haystack, const char *needle, size_t len);
int      tci_atoi(const char *str);

/* output */
int      tci_printf(const char *fmt, ...);

/* reader */
char    *tci_getline(int fd);

#endif
