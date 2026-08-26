NAME    = libtci.a
UTIL    = libtciutil.a
CC      = gcc
CFLAGS  = -Wall -Wextra -g -std=c99 -D BUFFER_SIZE=32
AR      = ar rcs

SRCS        = tci_memset.c tci_memcpy.c tci_memmove.c tci_memchr.c tci_bzero.c \
              tci_calloc.c \
              tci_isascii.c tci_isalpha.c tci_isdigit.c tci_isalnum.c tci_isspace.c \
              tci_isupper.c tci_islower.c tci_isprint.c \
              tci_toupper.c tci_tolower.c \
              tci_strlen.c tci_strcpy.c tci_strncpy.c \
              tci_strlcpy.c tci_strlcat.c \
              tci_strcmp.c tci_strncmp.c \
              tci_strchr.c tci_strrchr.c \
              tci_strdup.c tci_strndup.c tci_strnstr.c tci_atoi.c \
              tci_printf.c \
              tci_getline.c

UTIL_SRCS   = tciu_split.c

OBJS        = $(SRCS:.c=.o)
UTIL_OBJS   = $(UTIL_SRCS:.c=.o)

.PHONY: all clean fclean re

all: $(NAME) $(UTIL)

$(NAME): $(OBJS)
	$(AR) $(NAME) $(OBJS)

$(UTIL): $(UTIL_OBJS)
	$(AR) $(UTIL) $(UTIL_OBJS)

%.o: %.c libtci.h
	$(CC) $(CFLAGS) -c $< -o $@

tciu_split.o: tciu_split.c libtciutil.h libtci.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(UTIL_OBJS)

fclean: clean
	rm -f $(NAME) $(UTIL)

re: fclean all
