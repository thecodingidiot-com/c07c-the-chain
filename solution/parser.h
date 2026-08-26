#ifndef PARSER_H
# define PARSER_H

# include "lexer.h"

typedef enum e_node_type
{
    NODE_CMD,
    NODE_PIPE,
    NODE_AND,
    NODE_OR,
    NODE_SUBSHELL
}   t_node_type;

typedef struct s_redir
{
    t_tok_type      type;
    char            *target;
    struct s_redir  *next;
}   t_redir;

typedef struct s_node
{
    t_node_type     type;
    char            **argv;    /* NODE_CMD only */
    t_redir         *redirs;   /* NODE_CMD only */
    struct s_node   *left;
    struct s_node   *right;
}   t_node;

/* Returns NULL on a syntax error (also frees `tokens` in that case).
 * On success, the caller owns both the returned tree and `tokens` remains
 * owned by the caller too (parse() does not free it — main.c does, once
 * after both lexing and parsing are done). */
t_node  *parse(t_token *tokens);
void    free_node(t_node *node);

#endif
