#ifndef HANDLE_OPT_H
#define HANDLE_OPT_H

#include <err.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../server/server.h"
#include "config/config.h"

enum ACTION
{
    ERROR,
    NONE,
    START,
    STOP,
    RELOAD,
    RESTART = 5,
};

struct opt
{
    enum ACTION action;
    struct config *config;
};

struct opt handle_opt(int argc, char **argv);

#endif /* HANDLE_OPT_H */
