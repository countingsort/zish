#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_KAWAII_SMILEYS 5
static char *kawaii_smileys[5] = {
    "(▰˘◡˘▰)",
    "♥‿♥",
    "(✿ ♥‿♥)",
    ".ʕʘ‿ʘʔ.",
    "≧◡≦"
};

enum status_code {
    STAT_NORMAL,
    STAT_EXIT
};

static void zish_loop(void);

int main(void)
{
    // Load stuff
    srand(time(NULL));

    // REPL
    zish_loop();

    // Cleanup

    return EXIT_SUCCESS;
}

static void zish_loop(void)
{
    char  *line = NULL;
    char **args = NULL;
    enum status_code status = STAT_NORMAL;

    do {
        int kawaii_smiley_index = rand() % NUM_KAWAII_SMILEYS;
        printf(
            "\033[38;5;057mAwaiting your command, senpai. \033[38;5;197m%s \033[38;5;255m",
            kawaii_smileys[kawaii_smiley_index]
        );
        getc(stdin);
    } while (status != STAT_EXIT);
}

