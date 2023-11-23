#include "handle_opt.h"

static enum ACTION which_action(char *str)
{
    struct tmp
    {
        enum ACTION action;
        char *action_name;
    };
    static struct tmp token[] = { { START, "start" },
                                  { STOP, "stop" },
                                  { RELOAD, "reload" },
                                  { RESTART, "restart" } };
    for (size_t i = 0; i < sizeof(token) / sizeof(*token); i++)
        if (!strcmp(token[i].action_name, str))
            return token[i].action;

    fprintf(stderr,
            "./httpd [--dry-run] [-a (start | stop | reload | "
            "restart)] server.conf\n");
    exit(1);
}

struct opt handle_opt(int argc, char **argv)
{
    int dry_run = 0;
    int arg_expected = 0;
    char *path = NULL;
    struct opt opt = { NONE, NULL };

    for (int i = 0; i < argc; i++)
    {
        if (arg_expected)
        {
            if (opt.action != NONE)
            {
                fprintf(stderr,
                        "./httpd [--dry-run] [-a (start | stop | reload | "
                        "restart)] server.conf\n");
                exit(1);
            }

            opt.action = which_action(argv[i]);
            arg_expected = 0;
        }
        else if (!strcmp(argv[i], "--dry-run"))
            dry_run = 1;
        else if (!strcmp(argv[i], "-a"))
            arg_expected = 1;
        else if (path == NULL)
            path = argv[i];
        else
            exit(1);
    }
    opt.config = parse_configuration(path);
    set_path(path);
    if (opt.config == NULL)
    {
        errx(2, "Error on the config file");
    }
    if (dry_run)
    {
        config_destroy(opt.config);
        exit(0);
    }
    return opt;
}
