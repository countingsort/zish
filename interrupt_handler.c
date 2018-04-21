#include "interrupt_handler.h"

#include <stdio.h>
#include <stdlib.h>

#include <sys/signal.h>

#include <readline/readline.h>

/**
* The SIGINT interrupt handler.
* Prompts for the next command and discards everything.
*/
static void zish_interrupt_handler(int signo);

static void zish_interrupt_handler(int signo)
{
    if (signo == SIGINT) {
        printf("\n\033[38;5;12mIf you wanna go, try `exit`, onii-chan.\033[38;5;0m\n");
        rl_on_new_line();
        rl_replace_line("", 0);
        rl_redisplay();
    }

    zish_register_interrupt_handler();
}

void zish_register_interrupt_handler(void)
{
    if (signal(SIGINT, &zish_interrupt_handler) == SIG_ERR) {
        perror("zish");
        exit(EXIT_FAILURE);
    }
}

