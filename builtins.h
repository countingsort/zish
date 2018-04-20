#ifndef BUILTINS_H
#define BUILTINS_H

/**
 * Builtin function type
 */
typedef enum status_code (*builtin_func_t)(int, char**);

#define ZISH_NUM_BUILTINS 6

/**
* Builtin command names
*/
extern char *builtin_str[ZISH_NUM_BUILTINS];

/**
* Builtin command functions
*/
extern builtin_func_t builtin_func[ZISH_NUM_BUILTINS];

#endif /* BUILTINS_H */

