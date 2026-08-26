#ifndef LEXER_H
# define LEXER_H

typedef enum e_tok_type
{
    TOK_WORD,
    TOK_PIPE,
    TOK_REDIR_IN,
    TOK_REDIR_OUT,
    TOK_REDIR_APPEND,
    TOK_HEREDOC,
    TOK_AND,
    TOK_OR,
    TOK_LPAREN,
    TOK_RPAREN
}   t_tok_type;

typedef struct s_token
{
    t_tok_type      type;
    char            *value;
    struct s_token  *next;
}   t_token;

t_token *lex(char const *line);
void    free_tokens(t_token *tokens);

#endif
