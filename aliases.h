#ifndef ALIASES_H
#define ALIASES_H

#include "execute.h"

/**
 * Contains an alias and the command associated with it
 */
struct alias {
    char *name;
    char *command;
};

/**
 * NULL terminated list of aliases
 */
extern struct alias **aliases;

/**
 * Builtin: defines an alias
 *
 * @returns status of the command
 */
extern enum status_code zish_define_alias(int argc, char **argv);

#endif /* ALIASES_H */

