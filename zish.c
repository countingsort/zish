#include "builtins.h"
#include "interrupt_handler.h"
#include "execute.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <readline/readline.h>
#include <readline/history.h>

/**
 * Initializes everything needed
 */
static void zish_initialize(void);

/**
 * Unintializes everythin that was needed
 */
static void zish_cleanup(void);

/**
 * Touches a file. Kinda sexual
 */
static void zish_touch(const char *path);

/**
 * Name of the file used to save the history
 */
static const char *history_file = ".zish_history";

/**
 * Name of the zishrc
 */
static const char *config_file = ".zishrc";

int main(void)
{
    zish_initialize();
    zish_repl();
    zish_cleanup();

    return EXIT_SUCCESS;
}

static void zish_initialize(void)
{
    zish_register_interrupt_handler();

    char *prompt = getenv("PS1");
    if (!prompt)
        setenv("PS1", "$ ", false);

    char *home_path = getenv("HOME");
    int home_path_size = strlen(home_path);

    history_full_path = malloc((home_path_size + strlen(history_file) + 2) * sizeof(*history_full_path));
    strcpy(history_full_path, home_path);
    strcat(history_full_path, "/");
    strcat(history_full_path, history_file);

    char *config_full_path = malloc((home_path_size + strlen(history_file) + 2) * sizeof(*config_full_path));
    strcpy(config_full_path, home_path);
    strcat(config_full_path, "/");
    strcat(config_full_path, config_file);

    zish_touch(history_full_path);
    read_history(history_full_path);

    free(config_full_path);

    srand(time(NULL));

    aliases = calloc(1, sizeof(*aliases));
}

static void zish_cleanup(void)
{
    free(history_full_path);

    for (size_t i = 0; aliases[i]; ++i) {
        free(aliases[i]->name);
        free(aliases[i]->command);
        free(aliases[i]);
    }

    free(aliases);
}

static void zish_touch(const char *path)
{
    int fd = open(path, O_RDWR | O_CREAT | O_NONBLOCK | O_NOCTTY, 0666);
    if (fd < 0) {
        fprintf(stderr, "zish: Can't open history file.\n");
        exit(EXIT_FAILURE);
    }
    close(fd);
}
