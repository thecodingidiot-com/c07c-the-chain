CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -std=c99 -I libtci
LDFLAGS =
SRCS    = main.c builtins.c exec.c lexer.c parser.c
OBJS    = $(SRCS:.c=.o)
LIBS    = libtci/libtci.a libtci/libtciutil.a
NAME    = c07shell

.PHONY: all clean fclean re

all: $(NAME)

$(NAME): $(OBJS) $(LIBS)
	$(CC) $(OBJS) $(LIBS) $(LDFLAGS) -o $(NAME)

$(LIBS):
	$(MAKE) -C libtci

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(MAKE) -C libtci clean
	rm -f $(OBJS)

fclean: clean
	$(MAKE) -C libtci fclean
	rm -f $(NAME)

re: fclean all
