#include <stdlib.h>
#include "libtci.h"
#include "parser.h"

typedef struct s_argbuf
{
    char    **data;
    size_t  len;
    size_t  cap;
}   t_argbuf;

static void argbuf_init(t_argbuf *b)
{
    b->cap = 8;
    b->len = 0;
    b->data = tci_calloc(b->cap, sizeof(char *));
}

static void argbuf_push(t_argbuf *b, char *word)
{
    char    **bigger;
    size_t  i;

    if (b->len + 2 >= b->cap)
    {
        bigger = tci_calloc(b->cap * 2, sizeof(char *));
        i = 0;
        while (i < b->len)
        {
            bigger[i] = b->data[i];
            i++;
        }
        free(b->data);
        b->data = bigger;
        b->cap *= 2;
    }
    b->data[b->len] = word;
    b->len++;
    b->data[b->len] = NULL;
}

static int is_redir_tok(t_tok_type type)
{
    return (type == TOK_REDIR_IN || type == TOK_REDIR_OUT
        || type == TOK_REDIR_APPEND || type == TOK_HEREDOC);
}

static t_redir *new_redir(t_tok_type type, char *target)
{
    t_redir *r;

    r = tci_calloc(1, sizeof(t_redir));
    r->type = type;
    r->target = tci_strdup(target);
    r->next = NULL;
    return (r);
}

static void append_redir(t_redir **head, t_redir **tail, t_redir *r)
{
    if (!*head)
        *head = r;
    else
        (*tail)->next = r;
    *tail = r;
}

static t_node *new_cmd_node(char **argv, t_redir *redirs)
{
    t_node *node;

    node = tci_calloc(1, sizeof(t_node));
    node->type = NODE_CMD;
    node->argv = argv;
    node->redirs = redirs;
    return (node);
}

static t_node *new_binary_node(t_node_type type, t_node *left, t_node *right)
{
    t_node *node;

    node = tci_calloc(1, sizeof(t_node));
    node->type = type;
    node->left = left;
    node->right = right;
    return (node);
}

/* A simple command is one or more WORDs and redirections, in any order
 * (redirections may appear before, after, or between the words -- same as
 * a real shell: `> out.txt echo hi` is legal). Stops at end-of-tokens or
 * at an operator token (|, &&, ||, ')') it doesn't consume itself. */
static t_node *parse_simple_command(t_token **cur)
{
    t_argbuf    args;
    t_redir     *rhead;
    t_redir     *rtail;

    argbuf_init(&args);
    rhead = NULL;
    rtail = NULL;
    while (*cur && ((*cur)->type == TOK_WORD || is_redir_tok((*cur)->type)))
    {
        if ((*cur)->type == TOK_WORD)
        {
            argbuf_push(&args, tci_strdup((*cur)->value));
            *cur = (*cur)->next;
        }
        else
        {
            if (!(*cur)->next || (*cur)->next->type != TOK_WORD)
                return (NULL);
            append_redir(&rhead, &rtail, new_redir((*cur)->type, (*cur)->next->value));
            *cur = (*cur)->next->next;
        }
    }
    if (args.len == 0 && !rhead)
        return (NULL);
    return (new_cmd_node(args.data, rhead));
}

static t_node   *parse_and_or(t_token **cur);

/* "(" and_or ")"  |  simple_command */
static t_node *parse_unit(t_token **cur)
{
    t_node  *inner;

    if (*cur && (*cur)->type == TOK_LPAREN)
    {
        *cur = (*cur)->next;
        inner = parse_and_or(cur);
        if (!inner)
            return (NULL);
        if (!*cur || (*cur)->type != TOK_RPAREN)
            return (free_node(inner), NULL);
        *cur = (*cur)->next;
        return (new_binary_node(NODE_SUBSHELL, inner, NULL));
    }
    return (parse_simple_command(cur));
}

/* unit ( "|" unit )* -- left-associative */
static t_node *parse_pipeline(t_token **cur)
{
    t_node  *left;
    t_node  *right;

    left = parse_unit(cur);
    if (!left)
        return (NULL);
    while (*cur && (*cur)->type == TOK_PIPE)
    {
        *cur = (*cur)->next;
        right = parse_unit(cur);
        if (!right)
            return (free_node(left), NULL);
        left = new_binary_node(NODE_PIPE, left, right);
    }
    return (left);
}

/* pipeline ( ("&&"|"||") pipeline )* -- left-associative */
static t_node *parse_and_or(t_token **cur)
{
    t_node      *left;
    t_node      *right;
    t_node_type type;

    left = parse_pipeline(cur);
    if (!left)
        return (NULL);
    while (*cur && ((*cur)->type == TOK_AND || (*cur)->type == TOK_OR))
    {
        type = (*cur)->type == TOK_AND ? NODE_AND : NODE_OR;
        *cur = (*cur)->next;
        right = parse_pipeline(cur);
        if (!right)
            return (free_node(left), NULL);
        left = new_binary_node(type, left, right);
    }
    return (left);
}

t_node *parse(t_token *tokens)
{
    t_token *cur;
    t_node  *root;

    cur = tokens;
    root = parse_and_or(&cur);
    if (!root || cur != NULL)
    {
        if (root)
            free_node(root);
        return (NULL);
    }
    return (root);
}

static void free_redirs(t_redir *r)
{
    t_redir *next;

    while (r)
    {
        next = r->next;
        free(r->target);
        free(r);
        r = next;
    }
}

void free_node(t_node *node)
{
    int i;

    if (!node)
        return;
    if (node->type == NODE_CMD)
    {
        i = 0;
        while (node->argv && node->argv[i])
        {
            free(node->argv[i]);
            i++;
        }
        free(node->argv);
        free_redirs(node->redirs);
    }
    free_node(node->left);
    free_node(node->right);
    free(node);
}
