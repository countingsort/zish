#include "aliases.h"

#include "execute.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct alias **aliases = NULL;

enum status_code zish_define_alias(int argc, char **argv)
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
        perror("zish");
        return STAT_FAILURE;
    }

    new_alias->name    = strdup(argv[1]);
    new_alias->command = strdup(argv[2]);

    aliases = realloc(aliases, (i + 2) * sizeof(struct alias));
    if (!aliases) {
        perror("zish");
        exit(EXIT_FAILURE);
    }

    aliases[i]   = new_alias;
    aliases[i+1] = NULL;

    return STAT_SUCCESS;
}

