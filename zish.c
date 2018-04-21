#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <fcntl.h>
#include <pwd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <readline/readline.h>
#include <readline/history.h>

/**
* Contains an alias and the the commant associated with it
*/
struct alias {
    char *name;
    char *command;
};

/**
* Status of a command
*/
enum status_code {
    STAT_SUCCESS,
    STAT_FAILURE,
    STAT_EXIT
};

/** * Initializes everything needed
*/
static void 💩_initialize(void);

/**
* Uninitializes everthing not needed anymore
*/
static void 💩_cleanup(void);

/**
* The actual loop
*/
static void 💩_repl(void);

/**
* Get a line from a file
*
* @returns owning pointer to the line
*/
static char *💩_getline(FILE *file);

/**
* Loads a config file, basically executing it line by line
*/
static void 💩_load_config(const char *path);

/**
* Splits a line into different arguments
*
* @returns pointer to the char* array
*/
static char **💩_split_line(char *line, int *num_args);

/**
* Returns the next token in the line, similar to strtok
*
* @returns the next token
*/
static char *💩_linetok(char *line);

/**
* Own implementation of rawmemchr
*
* @returns pointer to the first occurance of c
*/
static void *💩_rawmemchr(const void *s, char c);

/**
* Execute a given command
*
* @returns status of the command
*/
static enum status_code 💩_exec(char **args, int num_args);

/**
* Launches a binary
*
* @returns status of the command
*/
static enum status_code 💩_launch(char **args);

/**
* Touches a given file
*/
static void 💩_touch(const char *path);

/**
* Builtin: change into a directory
*
* @returns status of the command
*/
static enum status_code 💩_cd(int argc, char **argv);

/**
* Builtin: print help
*
* @returns status of the command
*/
static enum status_code 💩_help(int argc, char **argv);

/**
* Builtin: exit the shell
*
* @returns STAT_EXIT
*/
static enum status_code 💩_exit(int argc, char **argv);

/**
* Builtin: defines an alias
*
* @returns status of the command
*/
static enum status_code 💩_define_alias(int argc, char **argv);

/**
* Builtin: assigns a value to a variable
*
* @returns status of the command
*/
static enum status_code 💩_assign_variable(int argc, char **argv);

/**
* Builtin: source a file
*
* @returns status of the command
*/
static enum status_code 💩_source_file(int argc, char **argv);

/**
* Registers the SIGINT interrupt handler
*/
static void 💩_register_interrupt_handler(void);

/**
* The SIGINT interrupt handler.
* Prompts for the next command and discards everything.
*/
static void 💩_interrupt_handler(int signo);

/**
* The name of the history file
*/
static const char *history_file = ".💩_history";

/**
* Full path to the history file
*/
static char *history_full_path  = NULL;

/**
* Name of the config file
*/
static const char *config_file = ".💩rc";

/**
* Array of all kawaii smileys
*/
#define 💩_NUM_KAWAII_SMILEYS 6
static const char *kawaii_smileys[💩_NUM_KAWAII_SMILEYS] = {
    "(▰˘◡˘▰)",
    "♥‿♥",
    "(✿ ♥‿♥)",
    ".ʕʘ‿ʘʔ.",
    "◎[▪‿▪]◎",
    "≧◡≦"
};

/**
* NULL terminated list of aliases
*/
static struct alias **aliases = NULL;

/**
* Builtin command names
*/
#define 💩_NUM_BUILTINS 6
static char *builtin_str[💩_NUM_BUILTINS] = {
    "cd",
    "help",
    "exit",
    "alias",
    "let",
    "source"
};

/**
* Builtin command functions
*/
static enum status_code (*builtin_func[💩_NUM_BUILTINS])(int, char **) = {
    &💩_cd,
    &💩_help,
    &💩_exit,
    &💩_define_alias,
    &💩_assign_variable,
    &💩_source_file
};

int main(void)
{
    💩_initialize();

    💩_repl();

    💩_cleanup();

    return EXIT_SUCCESS;
}

static void 💩_initialize(void)
{
    💩_register_interrupt_handler();

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

    💩_touch(history_full_path);
    read_history(history_full_path);

    srand(time(NULL));

    aliases   = calloc(1, sizeof(*aliases));

    if (access(config_full_path, F_OK) != -1) {
        💩_load_config(config_full_path);
    }
}

static void 💩_cleanup(void)
{
    free(history_full_path);

    for (size_t i = 0; aliases[i]; ++i) {
        free(aliases[i]->name);
        free(aliases[i]->command);
        free(aliases[i]);
    }

    free(aliases);
}

static void 💩_repl(void)
{
    char  *line = NULL;
    char **args = NULL;
    enum status_code status = STAT_SUCCESS;

    do {
        // Get input
        int kawaii_smiley_index = rand() % 💩_NUM_KAWAII_SMILEYS;
        char prompt[70 + sizeof(kawaii_smileys[kawaii_smiley_index])];

        if (status == STAT_SUCCESS) {
            sprintf(
                prompt,
                "\033[38;5;12mAwaiting your command, senpai. \033[38;5;13m%s \033[0m",
                kawaii_smileys[kawaii_smiley_index]
            );
        } else {
            strcpy(prompt, "\033[38;5;13mSomethwing went wrong, gome... \033[0m");
        }

        line = readline(prompt);
        if (!line || strlen(line) == 0)
            continue;

        if (!isspace(line[0]))
            add_history(line);
        write_history(history_full_path);

        int num_args;
        args = 💩_split_line(line, &num_args);

        status = 💩_exec(args, num_args);

        free(line);
        free(args);
    } while (status != STAT_EXIT);
}

#define 💩_LINE_BUFSIZE 256
static char *💩_getline(FILE *file)
{
    char  *line    = malloc(💩_LINE_BUFSIZE);
    char  *linep   = line;
    size_t lenmax = 💩_LINE_BUFSIZE;
    size_t len    = lenmax;
    int    c;

    if (!line) {
        return NULL;
    }

    for (;;) {
        c = fgetc(file);
        if (c == EOF) {
            *line = '\0';
            return linep;
        }

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

static void 💩_load_config(const char *path)
{
    char *line   = NULL;
    char **args  = NULL;
    enum status_code status = STAT_SUCCESS;

    FILE *config = fopen(path, "r");
    if (!config) {
        perror("💩: failed to load config");
        exit(EXIT_FAILURE);
    }

    while (!feof(config)) {
        line = 💩_getline(config);
        if (!line) {
            perror("💩");
            exit(EXIT_FAILURE);
        }

        if (strlen(line) == 0)
            continue;

        int num_args;
        args = 💩_split_line(line, &num_args);

        status = 💩_exec(args, num_args);

        free(line);
        free(args);

        if (status == STAT_EXIT)
            return;
    }
}

#define 💩_TOKEN_BUFSIZE 64
static char **💩_split_line(char *line, int *num_args)
{
    int    bufsize = 💩_TOKEN_BUFSIZE;
    int    pos     = 0;
    char **tokens  = malloc(bufsize * sizeof(*tokens));
    char  *token   = NULL;

    if (!tokens) {
        perror("💩");
    }

    token = 💩_linetok(line);
    while (token) {
        tokens[pos] = token;
        ++pos;

        if (pos >= bufsize) {
            bufsize *= 3;
            bufsize /= 2;
            tokens   = realloc(tokens, bufsize * sizeof(*tokens));

            if (!tokens) {
                perror("💩");
                exit(EXIT_FAILURE);
            }
        }

        token = 💩_linetok(NULL);
    }

    *num_args = pos;
    tokens[pos] = NULL;
    return tokens;
}

#define 💩_WHITESPACE " \t\n"
static char *💩_linetok(char *line)
{
    static char *old_line;
    char *token;

    if (line == NULL) {
        line = old_line;
    }

    line += strspn(line, 💩_WHITESPACE);
    if (*line == '\0') {
        old_line = line;
        return NULL;
    }

    token = line;
    line = strpbrk(token, 💩_WHITESPACE);
    if (line == NULL) {
        old_line = 💩_rawmemchr(token, '\0');
    } else {
        old_line = line + 1;
        *line = '\0';
    }

    return token;
}

static void *💩_rawmemchr(const void *s, char c)
{
    const char *str = s;
    while (*str != c) {
        ++str;
    }

    return (void*)str;
}

static enum status_code 💩_exec(char **args, int num_args)
{
    for (size_t i = 0; aliases[i]; ++i) {
        if (strcmp(args[0], aliases[i]->name) == 0) {
            args[0] = aliases[i]->command;
        }
    }

    for (size_t i = 0; i < 💩_NUM_BUILTINS; ++i) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(num_args, args);
        }
    }

    return 💩_launch(args);
}

static enum status_code 💩_launch(char **args)
{
    pid_t pid;

    int status;

    pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("💩");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("💩");
    } else {
        // Parent process
        do {
           waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    if (status) {
        return STAT_FAILURE;
    } else {
        return STAT_SUCCESS;
    }
}

static void 💩_touch(const char *path)
{
    int fd = open(path, O_RDWR | O_CREAT | O_NONBLOCK | O_NOCTTY, 0666);
    if (fd < 0) {
        fprintf(stderr, "💩: Can't open history file.\n");
        exit(EXIT_FAILURE);
    }
    close(fd);
}

/*
  Builtins
 */
static enum status_code 💩_cd(int argc, char **argv)
{
    char *dir = NULL;
    if (argc < 2) {
        dir = getenv("HOME");
    } else {
        dir = argv[1];
    }

    if (chdir(dir) != 0) {
        perror("💩");
    }

    return STAT_SUCCESS;
}

static enum status_code 💩_help(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    printf(
        "💩 is a shell\n"
        "(c) Ahmet Alkan Akarsu and Niclas Meyer\n\n"
        "Twype the pwogwam name and then pwess enter, onii-chan. (^._.^)~\n"
        "Builtwins:\n"
    );

    for (size_t i = 0; i < 💩_NUM_BUILTINS; ++i) {
        printf("  %s\n", builtin_str[i]);
    }

    printf("Use `man` for more inwos on other pwogwams.\n");

    return STAT_SUCCESS;
}

static enum status_code 💩_exit(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    printf("Sayounara, Onii-chan! (._.)\n");
    return STAT_EXIT;
}

static enum status_code 💩_define_alias(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "💩: expected 2 arguments to `alias`\n");

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
        perror("💩");
        return STAT_FAILURE;
    }

    new_alias->name    = strdup(argv[1]);
    new_alias->command = strdup(argv[2]);

    aliases = realloc(aliases, (i + 2) * sizeof(struct alias));
    if (!aliases) {
        perror("💩");
        exit(EXIT_FAILURE);
    }

    aliases[i]   = new_alias;
    aliases[i+1] = NULL;

    return STAT_SUCCESS;
}

static enum status_code 💩_assign_variable(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "💩: expected 2 arguments to `let`\n");

        return STAT_FAILURE;
    }

    if (setenv(argv[1], argv[2], true) == -1) {
        perror("setenv");
        return STAT_FAILURE;
    }

    return STAT_SUCCESS;
}

static enum status_code 💩_source_file(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "💩: expected an argument to `source`\n");
        return STAT_FAILURE;
    }

    💩_load_config(argv[1]);
    return STAT_SUCCESS;
}

static void 💩_register_interrupt_handler(void)
{
    if (signal(SIGINT, &💩_interrupt_handler) == SIG_ERR) {
        perror("💩");
        exit(EXIT_FAILURE);
    }
}

static void 💩_interrupt_handler(int signo)
{
    if (signo == SIGINT) {
        printf("\n\033[38;5;12mIf you wanna go, try `exit`, onii-chan.\033[38;5;0m\n");
        rl_on_new_line();
        rl_replace_line("", 0);
        rl_redisplay();
    }

    💩_register_interrupt_handler();
}

