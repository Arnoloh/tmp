#ifndef DAEMON_H
#define DAEMON_H

#define _POSIX_SOURCE
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "config/config.h"
#include "handle_arg/handle_opt.h"
#include "server/server.h"

int action_handler(struct opt opt);

#endif /* DAEMON_H */
