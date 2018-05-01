#ifndef BUILTINS_H
#define BUILTINS_H

#include <stdlib.h>

#include "execute.h"

/**
 * Builtin function type
 */
typedef enum status_code (*builtin_func_t)(size_t num_args, char *args[num_args]);


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
extern enum status_code zish_define_alias(size_t argc, char *argv[argc]);

/**
 * Builtin: source a file
 *
 * @returns status of the command
 */
enum status_code zish_source_file(size_t argc, char **argv);

#endif /* BUILTINS_H */
