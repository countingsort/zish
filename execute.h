#ifndef EXECUTE_H
#define EXECUTE_H

/**
 * Status of a command
 */
enum status_code {
    STAT_SUCCESS,
    STAT_FAILURE,
    STAT_EXIT,
};

/**
 * Full path to the history
 */
extern char *history_full_path;

/**
 * Reads a command, executes it and loops a lot
 */
extern void zish_repl(void);

#endif /* EXECUTE H */
