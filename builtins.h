#ifndef BUILTINS_H
#define BUILTINS_H

#include "execute.h"

/**
 * Builtin function type
 */
typedef enum status_code (*builtin_func_t)(int, char**);


/**
 * Contains an alias and the command associated with it
 */
struct alias {
    char *name;
    char *command;
};

#define ZISH_NUM_BUILTINS 6

/**
* Builtin command names
*/
extern char *builtin_str[ZISH_NUM_BUILTINS];

/**
* Builtin command functions
*/
extern builtin_func_t builtin_func[ZISH_NUM_BUILTINS];

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

/**
 * Builtin: source a file
 *
 * @returns status of the command
 */
enum status_code zish_source_file(int argc, char **argv);

#endif /* BUILTINS_H */
