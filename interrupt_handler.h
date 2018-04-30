#ifndef INTERRUPT_HANDLER_H
#define INTERRUPT_HANDLER_H

#include <setjmp.h>

/**
* Registers the SIGINT interrupt handler
*/
void zish_register_interrupt_handler(void);

extern sigjmp_buf ctrlc_buf;

#endif /* INTERRUPT_HANDLER_H */
