#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_KAWAII_SMILEYS 6
static char *kawaii_smileys[NUM_KAWAII_SMILEYS] = {
    "(▰˘◡˘▰)",
    "♥‿♥",
    "(✿ ♥‿♥)",
    ".ʕʘ‿ʘʔ.",
    "◎[▪‿▪]◎",
    "≧◡≦"
};

enum status_code {
    STAT_NORMAL,
    STAT_EXIT
};

static void zish_repl(void);
static char *zish_get_line(void);
static char **zish_split_line(char *line);
static enum status_code(char **args);

int main(void)
{
    // Load stuff
    srand(time(NULL));

    // REPL
    zish_repl();

    // Cleanup

    return EXIT_SUCCESS;
}

static void zish_repl(void)
{
    char  *line = NULL;
    char **args = NULL;
    enum status_code status = STAT_NORMAL;

    do {
        int kawaii_smiley_index = rand() % NUM_KAWAII_SMILEYS;

        // Print prompt
        printf(
            "\033[38;5;057mAwaiting your command, senpai. \033[38;5;197m%s \033[38;5;255m",
            kawaii_smileys[kawaii_smiley_index]
        );

        // Get input
        line = zish_get_line();
        args = zish_split_line(line);

        status = zish_exec(args);

        free(line);
        free(args);

        getc(stdin);
    } while (status != STAT_EXIT);
}

#define ZISH_LINE_BUFSIZE 256
static char *zish_get_line(void)
{
    char  *line   = malloc(ZISH_LINE_BUFSIZE);
    char  *linep  = line;
    size_t lenmax = ALLOC_SIZE;
    size_t len    = lenmax;
    int    c;

    if (!line)
        return NULL;

    for (;;) {
        c = fgetc(stdin);
        if (c == EOF)
            break;

        if (--len == 0) {
            len = lenmax;
            lenmax *= 3;
            lenmax /= 2;
            char *linen = realloc(linep, lenmax);

            if (!linen) {
                free(linep);
                return NULL;
            }
            line  = linen + (line - linep);
            linep = linen;
        }

        if ((*line++ = c) == '\n')
            break;
    }
    line[-1] = '\0';
    return linep;
}

#define ZISH_TOKEN_BUFSIZE 64
#define ZISH_TOKEN_DELIMS " \t\n\r"
static char **zish_split_line(char *line)
{
    int    bufsize = ZISH_TOKEN_BUFSIZE;
    int    pos     = 0;
    char **tokens  = malloc(bufsize * sizeof(*tokens));
    char  *token   = NULL;

    if (!tokens) {
        fprintf(stderr, "zish: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, ZISH_TOKEN_DELIMS);
    while (token) {
        tokens[position] = token;
        ++pos;

        if (pos >= bufsize) {
            bufsize *= 3;
            bufsize /= 2;
            tokens = realloc(tokens, bufsize * sizeof(*tokens));

            if (!tokens) {
                fprintf(stderr, "zish: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, ZISH_TOKEN_DELIMS);
    }

    tokens[pos] = NULL;
    return tokens;
}

static enum status_code(char **args)
{
}

