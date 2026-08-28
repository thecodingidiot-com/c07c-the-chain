#include <unistd.h>
#include <stdlib.h>
#include "shell.h"

int is_builtin(char const *name)
{
    if (!name)
        return (0);
    return (tci_strcmp(name, "cd") == 0
        || tci_strcmp(name, "pwd") == 0
        || tci_strcmp(name, "exit") == 0);
}

static int builtin_cd(char **argv)
{
    char const *target;

    target = argv[1];
    if (!target)
        target = getenv("HOME");
    if (!target) {
        tci_printf("cd: no HOME set\n");
        return (1);
    }
    if (chdir(target) != 0) {
        tci_printf("cd: %s: No such file or directory\n", target);
        return (1);
    }
    return (0);
}

static int builtin_pwd(void)
{
    char    buf[4096];

    if (!getcwd(buf, sizeof(buf))) {
        tci_printf("pwd: error retrieving current directory\n");
        return (1);
    }
    tci_printf("%s\n", buf);
    return (0);
}

int run_builtin(t_shell *sh, char **argv)
{
    if (tci_strcmp(argv[0], "cd") == 0)
        return (builtin_cd(argv));
    if (tci_strcmp(argv[0], "pwd") == 0)
        return (builtin_pwd());
    if (tci_strcmp(argv[0], "exit") == 0) {
        sh->running = 0;
        if (argv[1])
            return (tci_atoi(argv[1]));
        return (sh->last_status);
    }
    return (1);
}
