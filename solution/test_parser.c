#include <stdio.h>
#include "libtci.h"
#include "lexer.h"
#include "parser.h"

static int g_pass = 0;
static int g_fail = 0;

static void check(char const *label, int got, int want)
{
    if (got == want)
    {
        printf("PASS  %s\n", label);
        g_pass++;
    }
    else
    {
        printf("FAIL  %s (got %d, want %d)\n", label, got, want);
        g_fail++;
    }
}

static void check_str(char const *label, char const *got, char const *want)
{
    int ok;

    ok = got && want && tci_strcmp(got, want) == 0;
    if (ok)
    {
        printf("PASS  %s\n", label);
        g_pass++;
    }
    else
    {
        printf("FAIL  %s (got %s, want %s)\n", label,
            got ? got : "(null)", want ? want : "(null)");
        g_fail++;
    }
}

static int argc_of(t_node *n)
{
    int i;

    i = 0;
    while (n->argv[i])
        i++;
    return (i);
}

int main(void)
{
    t_token *toks;
    t_node  *n;

    /* quoted argument: one word, not two */
    toks = lex("echo \"hi there\"");
    n = parse(toks);
    check("quoted arg: node type is NODE_CMD", n->type, NODE_CMD);
    check("quoted arg: argc is 2", argc_of(n), 2);
    check_str("quoted arg: argv[1] is 'hi there'", n->argv[1], "hi there");
    free_node(n);
    free_tokens(toks);

    /* single quotes: no escape processing inside */
    toks = lex("echo 'a\\b'");
    n = parse(toks);
    check_str("single-quote: literal backslash kept", n->argv[1], "a\\b");
    free_node(n);
    free_tokens(toks);

    /* pipe chain */
    toks = lex("a | b | c");
    n = parse(toks);
    check("pipe chain: root is NODE_PIPE", n->type, NODE_PIPE);
    check("pipe chain: root.right is a simple command", n->right->type, NODE_CMD);
    check_str("pipe chain: root.right.argv[0] is 'c'", n->right->argv[0], "c");
    check("pipe chain: root.left is also a pipe (3-stage)", n->left->type, NODE_PIPE);
    free_node(n);
    free_tokens(toks);

    /* each redirection form */
    toks = lex("cmd < in.txt > out.txt >> app.txt << EOF");
    n = parse(toks);
    check("redirs: type[0] is REDIR_IN", n->redirs->type, TOK_REDIR_IN);
    check("redirs: type[1] is REDIR_OUT", n->redirs->next->type, TOK_REDIR_OUT);
    check("redirs: type[2] is REDIR_APPEND", n->redirs->next->next->type, TOK_REDIR_APPEND);
    check("redirs: type[3] is HEREDOC", n->redirs->next->next->next->type, TOK_HEREDOC);
    check_str("redirs: heredoc delimiter is EOF", n->redirs->next->next->next->target, "EOF");
    free_node(n);
    free_tokens(toks);

    /* && / || sequence */
    toks = lex("a && b || c");
    n = parse(toks);
    check("and/or: root is NODE_OR (left-assoc, || binds last)", n->type, NODE_OR);
    check("and/or: root.left is NODE_AND", n->left->type, NODE_AND);
    free_node(n);
    free_tokens(toks);

    /* subshell group */
    toks = lex("(a | b)");
    n = parse(toks);
    check("subshell: root is NODE_SUBSHELL", n->type, NODE_SUBSHELL);
    check("subshell: inner is NODE_PIPE", n->left->type, NODE_PIPE);
    free_node(n);
    free_tokens(toks);

    /* unterminated quote is a real lex error */
    toks = lex("echo \"unterminated");
    check("unterminated quote: lex returns NULL", toks == NULL, 1);

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return (g_fail > 0);
}
