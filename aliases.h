#ifndef ALIASES H
#define ALIASES H

/**
 * Contains an alias and the command associated with it
 */
struct alias {
    char *name;
    char *command;
};

/**
 * Builtin: defines an alias
 *
 * @returns status of the command
 */
extern enum status_code zish_define_alias(int argc, char **argv);

/**
 * NULL terminated list of aliases
 */
extern struct alias **aliases = NULL;

#endif /* ALIASES H */
