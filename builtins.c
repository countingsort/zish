#include "builtins.h"

#include "aliases.h"
#include "execute.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

/**
 * Builtin: change into a directory
 *
 * @returns status of the command
 */
static enum status_code zish_cd(size_t argc, char *argv[argc]);

/**
 * Builtin: print help
 *
 * @returns status of the command
 */
static enum status_code zish_help(size_t argc, char *argv[argc]);

/**
 * Builtin: exit the shell
 *
 * @returns STAT_EXIT
 */
static enum status_code zish_exit(size_t argc, char *argv[argc]);

/**
 * Builtin: assigns a value to a variable
 *
 * @returns status of the command
 */
static enum status_code zish_assign_variable(size_t argc, char *argv[argc]);

char *builtin_str[ZISH_NUM_BUILTINS] = {
    "cd",
    "help",
    "exit",
    "alias",
    "let",
    "source",
};

builtin_func_t builtin_func[ZISH_NUM_BUILTINS] = {
    &zish_cd,
    &zish_help,
    &zish_exit,
    &zish_define_alias,
    &zish_assign_variable,
    &zish_source_file,
};

struct alias **aliases;

static enum status_code zish_cd(size_t argc, char *argv[argc])
{
    char *dir = NULL;
    if (argc < 2) {
        dir = getenv("HOME");
    } else {
        dir = argv[1];
    }

    if (chdir(dir) != 0) {
        perror("zish");
        free(dir);
        return STAT_FAILURE;
    }

    free(dir);

    return STAT_SUCCESS;
}

static enum status_code zish_help(size_t argc, char *argv[argc])
{
    (void)argc;
    (void)argv;

    printf(
        "zish is a shell\n"
        "(c) Ahmet Alkan Akarsu and Niclas Meyer\n\n"
        "Twype the pwogwam name and then pwess enter, onii-chan. (^._.^)~\n"
        "Builtwins:\n"
    );

    for (size_t i = 0; i < ZISH_NUM_BUILTINS; ++i) {
        printf("  %s\n", builtin_str[i]);
    }

    printf("Use `man` for more inwos on other pwogwams.\n");

    return STAT_SUCCESS;
}

static enum status_code zish_exit(size_t argc, char *argv[argc])
{
    (void)argc;
    (void)argv;

    printf("Sayounara, Onii-chan! (._.)\n");
    return STAT_EXIT;
}

enum status_code zish_define_alias(size_t argc, char *argv[argc])
{
    if (argc < 3) {
        fprintf(stderr, "zish: expected 2 arguments to `alias`\n");

        return STAT_FAILURE;
    }

    size_t i = 0;
    while (aliases[i]) {
        if (strcmp(argv[1], aliases[i]->name) == 0) {
            aliases[i]->command = strdup(argv[2]);
            return STAT_SUCCESS;
        }
        ++i;
    }

    struct alias *new_alias = malloc(sizeof(*new_alias));
    if (!new_alias) {
        perror("malloc");
        return STAT_FAILURE;
    }

    new_alias->name    = strdup(argv[1]);
    new_alias->command = strdup(argv[2]);

    struct alias **new_aliases = realloc(aliases, (i + 2) * sizeof(struct alias));
    if (!aliases) {
        perror("realloc");
        free(new_alias);
        return STAT_FAILURE;
    }

    aliases = new_aliases;

    aliases[i]   = new_alias;
    aliases[i+1] = NULL;

    return STAT_SUCCESS;
}

static enum status_code zish_assign_variable(size_t argc, char *argv[argc])
{
    if (argc < 3) {
        fprintf(stderr, "zish: expected 2 arguments to let\n");
        return STAT_FAILURE;
    }

    if (setenv(argv[1], argv[2], true) == -1) {
        perror("setenv");
        return STAT_FAILURE;
    }

    return STAT_SUCCESS;
}

enum status_code zish_source_file(size_t argc, char *argv[argc])
{
    (void)argc;
    (void)argv;
    fprintf(stderr, "zish: source isn't implemented yet\n");
    return EXIT_FAILURE;
}
