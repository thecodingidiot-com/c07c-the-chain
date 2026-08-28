#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "shell.h"
#include "lexer.h"
#include "parser.h"

static void trim_newline(char *line)
{
    size_t  len;

    len = tci_strlen(line);
    if (len > 0 && line[len - 1] == '\n')
        line[len - 1] = '\0';
}

static int is_blank(char const *line)
{
    while (*line) {
        if (!tci_isspace((unsigned char)*line))
            return (0);
        line++;
    }
    return (1);
}

static void run_line(t_shell *sh, char const *line)
{
    t_token *tokens;
    t_node  *root;

    if (is_blank(line))
        return;
    tokens = lex(line);
    if (!tokens) {
        fprintf(stderr, "c07shell: syntax error: unterminated quote\n");
        sh->last_status = 2;
        return;
    }
    root = parse(tokens);
    if (!root) {
        fprintf(stderr, "c07shell: syntax error\n");
        free_tokens(tokens);
        sh->last_status = 2;
        return;
    }
    sh->last_status = exec_node(sh, root);
    free_node(root);
    free_tokens(tokens);
}

int main(void)
{
    t_shell sh;
    char    *line;
    int     interactive;

    sh.envp = NULL;
    sh.last_status = 0;
    sh.running = 1;
    interactive = isatty(STDIN_FILENO);
    while (sh.running) {
        if (interactive)
            tci_printf("$ ");
        line = tci_getline(STDIN_FILENO);
        if (!line) {
            if (interactive)
                tci_printf("exit\n");
            break;
        }
        /* BUG (fixed at checkpoint D / c07d): no sigaction call anywhere in
         * this program. Ctrl+C uses the default SIGINT disposition, which
         * terminates this whole process -- not just a running child. */
        trim_newline(line);
        run_line(&sh, line);
        free(line);
    }
    return (sh.last_status);
}
