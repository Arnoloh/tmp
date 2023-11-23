#define _POSIX_C_SOURCE 200809L
#include "daemon.h"

static int create_pid_file(struct config *config)
{
    FILE *fd = fopen(config->pid_file, "w+");
    if (fd == NULL)
        return -1;
    fprintf(fd, "%i\n", getpid());
    fclose(fd);
    return 0;
}

static int delete_pid_file(struct config *config)
{
    return remove(config->pid_file);
}
static int read_pid(struct config *config)
{
    FILE *fd = fopen(config->pid_file, "r");
    if (fd == NULL)
        return -1;
    int pid = 0;
    fscanf(fd, "%i", &pid);
    fclose(fd);
    return pid;
}
static int create_daemon(struct config *config)
{
    pid_t pid;

    pid = fork();

    if (pid < 0)
        return -1;
    if (pid == 0)
    {
        if (create_pid_file(config))
            return -1;
        if (config->log_file == NULL)
            config->log_file = strdup("HTTPd.log");
        return 0;
    }
    return 1;
}

int action_handler(struct opt opt)
{
    int pid;
    switch (opt.action)
    {
    case START:
        if (read_pid(opt.config) != -1)
            return -1;
        return create_daemon(opt.config);
    case STOP:
        pid = read_pid(opt.config);
        if (pid == -1)
            return 1;
        kill(pid, SIGINT);
        delete_pid_file(opt.config);
        return 1;
    case RELOAD:
        pid = read_pid(opt.config);
        if (pid == -1)
            return -1;
        kill(pid, SIGUSR1);
        return 1;
    case RESTART:
        opt.action = STOP;
        action_handler(opt);
        opt.action = START;
        return action_handler(opt);
    default:
        return 0;
    }
}
