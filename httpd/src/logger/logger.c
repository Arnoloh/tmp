#include "logger.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/stat.h>

int file_exists(char *filename)
{
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

int give_correct_fd(struct config *config)
{
    int fd = -1;
    if (config->log_file != NULL)
    {
        fd = open(config->log_file, O_CREAT | O_APPEND | O_WRONLY, 0644);
    }
    else
        fd = 1;
    return fd;
}
static void translate_flag(struct string *str, int flag)
{
    switch (flag)
    {
    case FB:
        string_concat_str(str, " Forbidden ", 11);
        break;
    case NF:
        string_concat_str(str, " Not Found ", 11);
        break;
    case MNA:
        string_concat_str(str, " UNKNOWN ", 9);
        break;
    case BR:
        string_concat_str(str, " Bad Request ", 13);
        break;
    case VNS:
        string_concat_str(str, " HTTP Version Not Supported ", 28);
        break;
    default:
        string_concat_str(str, " OK ", 28);
        break;
    }
}

void close_log(struct server_config serv, struct config *config)
{
    if (config->log_file != NULL)
        close(serv.log_fd);
}

void greatefull_shutdown(struct server_config server, struct config *config)
{
    if (!config->log)
        return;
    struct string *log = string_create("", 0);
    put_time(log, 0);
    string_concat_str(log, " [", 2);

    string_concat_str(log, server.server_name->data, server.server_name->size);

    string_concat_str(log, "] Graceful shutdown",
                      strlen("] Graceful shutdown"));

    string_concat_str(log, "\n", 1);
    write(server.log_fd, log->data, log->size);
    string_destroy(log);
}

void logger_error_recv(struct config *config, int flag,
                       struct server_config server)
{
    if (!config->log)
        return;
    struct string *log = string_create("", 0);
    put_time(log, 0);
    string_concat_str(log, " [", 2);

    string_concat_str(log, server.server_name->data, server.server_name->size);
    string_concat_str(log, "] received", 10);

    char error[50];
    sprintf(error, "%d", flag);
    translate_flag(log, flag);

    string_concat_str(log, "from ", 5);
    string_concat_str(log, server.ip_client, strlen(server.ip_client));
    string_concat_str(log, "\n", 1);
    write(server.log_fd, log->data, log->size);
    string_destroy(log);
}

static void put_method(struct string *str, int method)
{
    switch (method)
    {
    case GET:
        string_concat_str(str, S_GET, strlen(S_GET));
        break;
    case HEAD:
        string_concat_str(str, S_HEAD, strlen(S_HEAD));
        break;
    default:
        string_concat_str(str, "UNKNOWN", strlen("UNKNOWN"));
        break;
    }
}

void logger_ok_recv(struct config *config, struct server_config server,
                    struct header *header)
{
    if (!config->log)
        return;
    struct string *log = string_create("", 0);
    put_time(log, 0);
    string_concat_str(log, " [", 2);

    string_concat_str(log, server.server_name->data, server.server_name->size);

    string_concat_str(log, "] received ", 11);
    put_method(log, header->method);
    string_concat_str(log, " on '", 5);
    string_concat_str(log, header->target->data, header->target->size);
    string_concat_str(log, "' ", 2);

    string_concat_str(log, "from ", 5);
    string_concat_str(log, server.ip_client, strlen(server.ip_client));
    string_concat_str(log, "\n", 1);
    write(server.log_fd, log->data, log->size);
    string_destroy(log);
}

static void put_on(struct string *log, struct header *header)
{
    if (header == NULL || header->target == NULL)
        return;
    string_concat_str(log, " on '", strlen(" on '"));
    string_concat_str(log, header->target->data, header->target->size);
    string_concat_str(log, "'", 1);
}

void logger_send(struct config *config, int flag, struct server_config server,
                 struct header *header)
{
    if (!config->log)
        return;

    struct string *log = string_create("", 0);
    put_time(log, 0);
    string_concat_str(log, " [", 2);

    string_concat_str(log, server.server_name->data, server.server_name->size);

    char error[50];
    sprintf(error, "%d", flag);
    string_concat_str(log, "] responding with ", strlen("] responding with "));
    string_concat_str(log, error, strlen(error));
    string_concat_str(log, " to ", strlen(" to "));
    string_concat_str(log, server.ip_client, strlen(server.ip_client));

    if (flag != BR)
    {
        string_concat_str(log, " for ", strlen(" for "));
        put_method(log, header->method);
        put_on(log, header);
    }
    string_concat_str(log, "\n", 1);
    write(server.log_fd, log->data, log->size);
    string_destroy(log);
}
