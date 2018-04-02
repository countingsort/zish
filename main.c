#include <stdio.h>
#include <stdlib.h>

void zish_loop(void);

int main(void)
{
    // Load stuff

    // REPL
    zish_loop();

    // Cleanup

    return EXIT_SUCCESS;
}

void zish_loop(void)
{
    char *line = NULL;
    char **args = NULL;
    int status;
}

