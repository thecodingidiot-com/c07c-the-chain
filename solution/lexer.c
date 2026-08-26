#include <stdlib.h>
#include "libtci.h"
#include "lexer.h"

typedef struct s_buf
{
    char    *data;
    size_t  len;
    size_t  cap;
}   t_buf;

static void buf_init(t_buf *b)
{
    b->cap = 32;
    b->len = 0;
    b->data = tci_calloc(b->cap, 1);
}

static void buf_push(t_buf *b, char c)
{
    char    *bigger;

    if (b->len + 2 >= b->cap)
    {
        bigger = tci_calloc(b->cap * 2, 1);
        tci_memcpy(bigger, b->data, b->len);
        free(b->data);
        b->data = bigger;
        b->cap *= 2;
    }
    b->data[b->len] = c;
    b->len++;
    b->data[b->len] = '\0';
}

static int is_operator_char(char c)
{
    return (c == '|' || c == '&' || c == '<' || c == '>'
        || c == '(' || c == ')');
}

/* Reads one WORD starting at line[*i], handling quotes and escapes.
 * A word can splice together quoted and unquoted segments (echo hi"there"
 * is one word, "hithere") -- the loop keeps going across quote boundaries,
 * only stopping at real whitespace/operators/end while unquoted.
 * Returns 0 on success, -1 on an unterminated quote. */
static int lex_word(char const *s, int *i, t_buf *out)
{
    while (s[*i] && !tci_isspace((unsigned char)s[*i])
        && !is_operator_char(s[*i]))
    {
        if (s[*i] == '\'')
        {
            (*i)++;
            while (s[*i] && s[*i] != '\'')
            {
                buf_push(out, s[*i]);
                (*i)++;
            }
            if (!s[*i])
                return (-1);
            (*i)++;
        }
        else if (s[*i] == '"')
        {
            (*i)++;
            while (s[*i] && s[*i] != '"')
            {
                if (s[*i] == '\\' && s[*i + 1] && tci_strchr("\"\\$`", s[*i + 1]))
                    (*i)++;
                buf_push(out, s[*i]);
                (*i)++;
            }
            if (!s[*i])
                return (-1);
            (*i)++;
        }
        else if (s[*i] == '\\' && s[*i + 1])
        {
            (*i)++;
            buf_push(out, s[*i]);
            (*i)++;
        }
        else
        {
            buf_push(out, s[*i]);
            (*i)++;
        }
    }
    return (0);
}

static t_token *new_token(t_tok_type type, char *value)
{
    t_token *tok;

    tok = tci_calloc(1, sizeof(t_token));
    tok->type = type;
    tok->value = value;
    tok->next = NULL;
    return (tok);
}

static void append(t_token **head, t_token **tail, t_token *tok)
{
    if (!*head)
        *head = tok;
    else
        (*tail)->next = tok;
    *tail = tok;
}

static t_token *lex_operator(char const *s, int *i)
{
    if (s[*i] == '&' && s[*i + 1] == '&')
    {
        *i += 2;
        return (new_token(TOK_AND, tci_strdup("&&")));
    }
    if (s[*i] == '|' && s[*i + 1] == '|')
    {
        *i += 2;
        return (new_token(TOK_OR, tci_strdup("||")));
    }
    if (s[*i] == '>' && s[*i + 1] == '>')
    {
        *i += 2;
        return (new_token(TOK_REDIR_APPEND, tci_strdup(">>")));
    }
    if (s[*i] == '<' && s[*i + 1] == '<')
    {
        *i += 2;
        return (new_token(TOK_HEREDOC, tci_strdup("<<")));
    }
    if (s[*i] == '|')
        return ((*i)++, new_token(TOK_PIPE, tci_strdup("|")));
    if (s[*i] == '<')
        return ((*i)++, new_token(TOK_REDIR_IN, tci_strdup("<")));
    if (s[*i] == '>')
        return ((*i)++, new_token(TOK_REDIR_OUT, tci_strdup(">")));
    if (s[*i] == '(')
        return ((*i)++, new_token(TOK_LPAREN, tci_strdup("(")));
    return ((*i)++, new_token(TOK_RPAREN, tci_strdup(")")));
}

t_token *lex(char const *line)
{
    t_token *head;
    t_token *tail;
    t_token *tok;
    int     i;
    t_buf   buf;

    head = NULL;
    tail = NULL;
    i = 0;
    while (line[i])
    {
        while (line[i] && tci_isspace((unsigned char)line[i]))
            i++;
        if (!line[i])
            break;
        if (is_operator_char(line[i]))
            tok = lex_operator(line, &i);
        else
        {
            buf_init(&buf);
            if (lex_word(line, &i, &buf) != 0)
            {
                free(buf.data);
                free_tokens(head);
                return (NULL);
            }
            tok = new_token(TOK_WORD, buf.data);
        }
        append(&head, &tail, tok);
    }
    return (head);
}

void free_tokens(t_token *tokens)
{
    t_token *next;

    while (tokens)
    {
        next = tokens->next;
        free(tokens->value);
        free(tokens);
        tokens = next;
    }
}
