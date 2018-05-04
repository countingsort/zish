#include "interrupt_handler.h"

#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/signal.h>

#include <readline/readline.h>

/**
* The SIGINT interrupt handler.
* Prompts for the next command and discards everything.
*/
static void zish_interrupt_handler(int signo);

sigjmp_buf ctrlc_buf;

static void zish_interrupt_handler(int signo)
{
    zish_register_interrupt_handler();
    if (signo == SIGINT) {
	siglongjmp(ctrlc_buf, 1);
    }
}

void zish_register_interrupt_handler(void)
{
    if (signal(SIGINT, &zish_interrupt_handler) == SIG_ERR) {
        perror("zish: signal()");
        exit(EXIT_FAILURE);
    }
}

