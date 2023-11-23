#define _POSIX_C_SOURCE 200809L
#include "config.h"

#include <fnmatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct config *config_global(FILE *stream, struct config *config);
static struct config *config_vhost(FILE *stream, struct config *config);
void config_destroy(struct config *config);

struct parser
{
    char *name;
    struct config *(*func)(FILE *stream, struct config *config);
};

static struct parser param[] = { { "\\[global\\]*", config_global },
                                 { "\\[\\[vhosts\\]\\]*", config_vhost } };

static char *sperate(char *str)
{
    strtok(str, " = ");
    char *tok = strtok(NULL, " = ");
    if (tok == NULL)
        return NULL;
    tok = strtok(tok, "\n");
    if (tok == NULL)
        return NULL;
    return strdup(tok);
}
static struct config *config_vhost(FILE *stream, struct config *config)
{
    if (config == NULL)
        return NULL;
    config->nb_servers += 1;
    config->servers = realloc(
        config->servers, config->nb_servers * sizeof(struct server_config));
    struct server_config serv_config = {
        NULL, NULL, NULL, NULL, NULL, { 0 }, -1
    };
    char *line = NULL;
    size_t len = 0;
    while (getline(&line, &len, stream) != -1)
    {
        if (line[0] == '\n')
            break;
        if (!fnmatch("server_name*", line, 0))
        {
            char *tok = sperate(line);
            serv_config.server_name = string_create(tok, strlen(tok));
            free(tok);
        }
        else if (!fnmatch("port*", line, 0))
        {
            serv_config.port = sperate(line);
        }
        else if (!fnmatch("ip*", line, 0))
            serv_config.ip = sperate(line);
        else if (!fnmatch("root_dir*", line, 0))
            serv_config.root_dir = sperate(line);
        else if (!fnmatch("default_file*", line, 0))
            serv_config.default_file = sperate(line);
    }
    free(line);
    if (serv_config.default_file == NULL)
        serv_config.default_file = strdup("index.html");
    config->servers[config->nb_servers - 1] = serv_config;
    if (serv_config.server_name == NULL || serv_config.port == NULL
        || serv_config.root_dir == NULL || serv_config.ip == NULL)
    {
        config_destroy(config);
        return NULL;
    }

    return config;
}

static struct config *config_global(FILE *stream, struct config *config)
{
    if (config != NULL)
    {
        config_destroy(config);
        return NULL;
    }
    char *line = NULL;
    size_t len = 0;
    config = calloc(1, sizeof(struct config));
    config->log = true;
    while (getline(&line, &len, stream) != -1)
    {
        if (line[0] == '\n')
            break;
        if (!fnmatch("log_file*", line, 0))
            config->log_file = sperate(line);
        else if (!fnmatch("log*", line, 0))
        {
            char *tok = sperate(line);
            if (!strcmp(tok, "false"))
                config->log = false;
            free(tok);
        }
        else if (!fnmatch("pid_file*", line, 0))
        {
            char *tok = sperate(line);
            if (tok == NULL)
                break;
            config->pid_file = tok;
        }
    }
    free(line);
    if (config->pid_file == NULL)
    {
        config_destroy(config);
        return NULL;
    }
    return config;
}

struct config *parse_configuration(const char *path)
{
    FILE *fd = fopen(path, "r");
    if (fd == NULL)
        return NULL;
    size_t nread;
    char *line = NULL;
    size_t len;
    struct config *config = NULL;
    while ((nread = getline(&line, &len, fd) != -1))
    {
        for (size_t i = 0; i < sizeof(param) / sizeof(*param); i++)
        {
            if (!fnmatch(param[i].name, line, 0))
            {
                config = param[i].func(fd, config);
                break;
            }
        }
        if (config == NULL)
            break;
    }
    fclose(fd);
    free(line);
    if (config != NULL && config->nb_servers == 0)
    {
        config_destroy(config);
        return NULL;
    }
    return config;
}

void config_destroy(struct config *config)
{
    if (config == NULL)
        return;
    for (size_t i = 0; i < config->nb_servers; i++)
    {
        struct server_config sc = config->servers[i];
        if (sc.server_name != NULL)
            string_destroy(sc.server_name);
        free(sc.port);
        free(sc.ip);
        free(sc.root_dir);
        free(sc.default_file);
    }
    free(config->pid_file);
    free(config->log_file);
    free(config->servers);
    free(config);
}
