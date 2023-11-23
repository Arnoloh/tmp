#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "config/config.h"
#include "daemon/daemon.h"
#include "handle_arg/handle_opt.h"
#include "server/server.h"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr,
                "%s [--dry-run] [-a (start | stop | reload | restart)] "
                "server.conf\n",
                argv[0]);
        return 1;
    }
    struct opt opt = handle_opt(argc - 1, argv + 1);
    int action = action_handler(opt);
    switch (action)
    {
    case -1:
        config_destroy(opt.config);
        return 1;
    case 1:
        config_destroy(opt.config);
        return 0;
    case 0:
        break;
    default:
        fprintf(stderr, "action not implemented yet\n");
        return 3;
    }
    struct sigaction sig;
    sig.sa_flags = 0;
    sig.sa_handler = &update_run;
    if (sigemptyset(&sig.sa_mask) < 0)
        return -1;
    if (sigaction(SIGINT, &sig, NULL) < 0 || sigaction(SIGUSR1, &sig, NULL))
        return -1;
    lunch_serverHTTPD(opt.config);
    return 0;
}
