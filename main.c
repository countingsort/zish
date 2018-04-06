#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <fcntl.h>
#include <pwd.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <readline/readline.h>
#include <readline/history.h>

// Somehow not included in string.h
extern char *strdup(const char *s);

struct alias {
    char *name;
    char *command;
};

enum status_code {
    STAT_SUCCESS,
    STAT_FAILURE,
    STAT_EXIT
};

static void zish_initialize(void);
static void zish_cleanup(void);
static void zish_repl(void);

static char *zish_getline(FILE *file);
static void zish_load_config(const char *path);

static char **zish_split_line(char *line, int *num_args);
static char *zish_linetok(char *line);
static void *zish_rawmemchr(const void *s, char c);
static enum status_code zish_exec(char **args, int num_args);
static enum status_code zish_launch(char **args);

static void zish_touch(const char *path);

static enum status_code zish_cd(int argc, char **argv);
static enum status_code zish_help(int argc, char **argv);
static enum status_code zish_exit(int argc, char **argv);
static enum status_code zish_define_alias(int argc, char **argv);

static void zish_register_interrupt_handler(void);
static void zish_interrupt_handler(int signo);

static const char *history_file = ".zish_history";
static char *history_full_path  = NULL;

static const char *config_file = ".zishrc";

#define ZISH_NUM_KAWAII_SMILEYS 6
static const char *kawaii_smileys[ZISH_NUM_KAWAII_SMILEYS] = {
    "(▰˘◡˘▰)",
    "♥‿♥",
    "(✿ ♥‿♥)",
    ".ʕʘ‿ʘʔ.",
    "◎[▪‿▪]◎",
    "≧◡≦"
};

static struct alias **aliases = NULL;

#define ZISH_NUM_BUILTINS 4
static char *builtin_str[ZISH_NUM_BUILTINS] = {
    "cd",
    "help",
    "exit",
    "alias"
};

static enum status_code (*builtin_func[ZISH_NUM_BUILTINS])(int, char **) = {
    &zish_cd,
    &zish_help,
    &zish_exit,
    &zish_define_alias
};

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

    struct passwd *pw = getpwuid(getuid());
    int home_path_size = strlen(pw->pw_dir);

    history_full_path = malloc((home_path_size + strlen(history_file) + 2) * sizeof(*history_full_path));
    strcpy(history_full_path, pw->pw_dir);
    strcat(history_full_path, "/");
    strcat(history_full_path, history_file);

    char *config_full_path = malloc((home_path_size + strlen(history_file) + 2) * sizeof(*config_full_path));
    strcpy(config_full_path, pw->pw_dir);
    strcat(config_full_path, "/");
    strcat(config_full_path, config_file);

    zish_touch(history_full_path);
    read_history(history_full_path);

    srand(time(NULL));

    aliases = calloc(1, sizeof(struct alias));

    zish_touch(config_full_path);
    zish_load_config(config_full_path);
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

static void zish_repl(void)
{
    char  *line = NULL;
    char **args = NULL;
    enum status_code status = STAT_SUCCESS;

    do {
        // Get input
        int kawaii_smiley_index = rand() % ZISH_NUM_KAWAII_SMILEYS;
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

        add_history(line);
        write_history(history_full_path);

        int num_args;
        args = zish_split_line(line, &num_args);

        status = zish_exec(args, num_args);

        free(line);
        free(args);
    } while (status != STAT_EXIT);
}

#define ZISH_LINE_BUFSIZE 256
static char *zish_getline(FILE *file)
{
    char  *line    = malloc(ZISH_LINE_BUFSIZE);
    char  *linep   = line;
    size_t lenmax = ZISH_LINE_BUFSIZE;
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

static void zish_load_config(const char *path)
{
    char *line   = NULL;
    char **args  = NULL;
    enum status_code status = STAT_SUCCESS;

    FILE *config = fopen(path, "r");
    if (!config) {
        perror("zish: failed to load config");
        exit(EXIT_FAILURE);
    }

    while (!feof(config)) {
        line = zish_getline(config);
        if (!line) {
            perror("zish");
            exit(EXIT_FAILURE);
        }

        if (strlen(line) == 0)
            continue;

        int num_args;
        args = zish_split_line(line, &num_args);

        status = zish_exec(args, num_args);

        free(line);
        free(args);

        if (status == STAT_EXIT)
            return;
    }
}

#define ZISH_TOKEN_BUFSIZE 64
static char **zish_split_line(char *line, int *num_args)
{
    int    bufsize = ZISH_TOKEN_BUFSIZE;
    int    pos     = 0;
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

#define ZISH_TOKEN_DELIMS " \t\n"
static char *zish_linetok(char *line)
{
    static char *old_line;
    char *token;

    if (line == NULL) {
        line = old_line;
    }

    line += strspn(line, ZISH_TOKEN_DELIMS);
    if (*line == '\0') {
        old_line = line;
        return NULL;
    }

    token = line;
    line = strpbrk(token, ZISH_TOKEN_DELIMS);
    if (line == NULL) {
        old_line = zish_rawmemchr(token, '\0');
    } else {
        *line = '\0';
        old_line = line + 1;
    }
    return token;
}

static void *zish_rawmemchr(const void *s, char c)
{
    const char *str = s;
    while (*str != c) {
        ++str;
    }

    return (void*)str;
}

static enum status_code zish_exec(char **args, int num_args)
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
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("zish");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("zish");
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

static void zish_touch(const char *path)
{
    int fd = open(path, O_RDWR | O_CREAT | O_NONBLOCK | O_NOCTTY, 0666);
    if (fd < 0) {
        fprintf(stderr, "Can't open history file.\n");
        exit(EXIT_FAILURE);
    }
    close(fd);
}

/*
  Builtins
 */
static enum status_code zish_cd(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "zish: expected argument to `cd`\n");
        return STAT_FAILURE;
    }

    if (chdir(argv[1]) != 0) {
        perror("zish");
    }

    return STAT_SUCCESS;
}

static enum status_code zish_help(int argc, char **argv)
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

static enum status_code zish_exit(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    printf("Sayounara, Onii-chan! (._.)\n");
    return STAT_EXIT;
}

static enum status_code zish_define_alias(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "zish: expected 2 arguments to `alias`\n");

        return STAT_FAILURE;
    }

    struct alias *new_alias = malloc(sizeof(*new_alias));
    if (!new_alias) {
        perror("zish");
        exit(EXIT_FAILURE);
    }

    new_alias->name    = strdup(argv[1]);
    new_alias->command = strdup(argv[2]);

    size_t i = 0;
    while (aliases[i]) {
        ++i;
    }

    aliases = realloc(aliases, (i + 2) * sizeof(struct alias));
    if (!aliases) {
        perror("zish");
        exit(EXIT_FAILURE);
    }

    aliases[i]   = new_alias;
    aliases[i+1] = NULL;

    return STAT_SUCCESS;
}

static void zish_register_interrupt_handler(void)
{
    if (signal(SIGINT, &zish_interrupt_handler) == SIG_ERR) {
        perror("zish");
        exit(EXIT_FAILURE);
    }
}

static void zish_interrupt_handler(int signo)
{
    if (signo == SIGINT) {
        printf("\n\033[38;5;12mIf you wanna go, try `exit`, onii-chan.\033[38;5;0m\n");
        rl_on_new_line();
        rl_replace_line("", 0);
        rl_redisplay();
    }

    zish_register_interrupt_handler();
}

