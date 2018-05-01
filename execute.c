#include "execute.h"

#include "aliases.h"
#include "builtins.h"
#include "interrupt_handler.h"

#include <stdio.h>
#include <stdlib.h>

#include <errno.h>

#include <sys/wait.h>
#include <unistd.h>

#include <readline/readline.h>
#include <readline/history.h>

/**
* Splits a line into different arguments
*
* @returns pointer to the char* array
*/
static char **zish_split_line(char *line, size_t *num_args);

/**
* Returns the next token in the line, similar to strtok
*
* @returns the next token
*/
static char *zish_linetok(char *line);

/**
* Own implementation of rawmemchr
*
* @returns pointer to the first occurance of c
*/
static void *zish_rawmemchr(void *s, int c);

/**
* Execute a given command
*
* @returns status of the command
*/
static enum status_code zish_exec(size_t num_args, char *args[num_args]);

/**
* Launches a binary
*
* @returns status of the command
*/
static enum status_code zish_launch(char **args);

char *history_full_path = NULL;

void zish_repl(void)
{
    char  *line = NULL;
    char **args = NULL;
    enum status_code status = STAT_SUCCESS;

    while (sigsetjmp(ctrlc_buf, 1) !=  0);

    do {
        // Get input
        char *prompt = getenv("PS1");
        line = readline(prompt);
        if (!line || strlen(line) == 0) {
            free(line);
            continue;
        }

        if (!isspace(line[0]))
            add_history(line);
        write_history(history_full_path);


        size_t num_args;
        args = zish_split_line(line, &num_args);

        status = zish_exec(num_args, args);

        free(line);
        free(args);
    } while (status != STAT_EXIT);
}

#define ZISH_TOKEN_BUFSIZE 64
static char **zish_split_line(char *line, size_t *num_args)
{
    size_t bufsize = ZISH_TOKEN_BUFSIZE;
    size_t    pos     = 0;
    char **tokens  = malloc(bufsize * sizeof(*tokens));
    char  *token   = NULL;

    if (!tokens) {
        perror("zish");
    }

    token = zish_linetok(line);
    while (token) {
        tokens[pos] = token;
        ++pos;

        if (pos >= bufsize) {
            bufsize *= 3;
            bufsize /= 2;
            tokens   = realloc(tokens, bufsize * sizeof(*tokens));

            if (!tokens) {
                perror("zish");
                exit(EXIT_FAILURE);
            }
        }

        token = zish_linetok(NULL);
    }

    *num_args = pos;
    tokens[pos] = NULL;
    return tokens;
}

#define ZISH_WHITESPACE " \t\n"
static char *zish_linetok(char *line)
{
    static char *old_line;
    char *token;

    if (line == NULL) {
        line = old_line;
    }

    line += strspn(line, ZISH_WHITESPACE);
    if (*line == '\0') {
        old_line = line;
        return NULL;
    }

    token = line;
    line = strpbrk(token, ZISH_WHITESPACE);
    if (line == NULL) {
        old_line = zish_rawmemchr(token, '\0');
    } else {
        old_line = line + 1;
        *line = '\0';
    }

    return token;
}

static void *zish_rawmemchr(void *s, int c)
{
    char *str = s;
    while (*str != (char)c) {
        ++str;
    }

    return str;
}

static enum status_code zish_exec(size_t num_args, char *args[num_args])
{
    for (size_t i = 0; aliases[i]; ++i) {
        if (strcmp(args[0], aliases[i]->name) == 0) {
            args[0] = aliases[i]->command;
        }
    }

    for (size_t i = 0; i < ZISH_NUM_BUILTINS; ++i) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(num_args, args);
        }
    }

    return zish_launch(args);
}

static enum status_code zish_launch(char **args)
{
    pid_t pid;

    int status;

    pid = fork();
    if (pid < 0) {
        // Error forking
        perror("zish: fork()");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            fprintf(stderr, "zish: execvp(%s)\n", args[0]);
            perror(args[0]);
            return STAT_FAILURE;
        }
        /* should not reach:  only returns on error */
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("fork");
    } else {
        // Parent process
        pid_t err;
        do {
           err = waitpid(pid, &status, WUNTRACED);
           /* if an waitpid() errored but not because of the parent being
            * interrupted, print it, but continue trying
            * TODO:  If parent is interrupted, kill child?
            */
           if (err != pid && err != EINTR) {
               perror("zish: waitpid()");
               continue;
           }
        } while (err != pid || /* status is valid */
                 (!WIFEXITED(status) && !WIFSIGNALED(status)));
    }

    if (status) {
        return STAT_FAILURE;
    } else {
        return STAT_SUCCESS;
    }
}
