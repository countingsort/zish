#include <stdlib.h>
#include <string.h>
#include <time.h>

static void zish_initialize(void);

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

    srand(time(NULL));

    aliases = calloc(1, sizeof(*aliases));

    if (access(config_full_path, F_OK) != -1) {
        zish_load_config(config_full_path);
    }
}

