#define _POSIX_C_SOURCE 200809L
#include "http.h"

static int connected = 0;

static void sigpipe_handler(int sig)
{
    if (sig == SIGPIPE)
        connected = 0;
}

void destroy_header(struct header *header);

static void reset_buffer(char *buf)
{
    for (size_t i = 0; i < BUFFER_SIZE; i++)
        buf[i] = '\0';
}

int how_many_length(struct string *str)
{
    for (size_t i = 0; i < str->size; i++)
    {
        if (!string_compare_n_start_str(
                str, "Content-Length: ", strlen("Content-Length: "), i))
        {
            char *copy = str->data + i;
            int len = -1;
            while (*copy != ':')
                copy++;
            copy++;
            while (*copy == ' ')
                copy++;
            sscanf(copy, "%i", &len);
            return len;
        }
    }
    return -1;
}
int check_host(struct server_config serv, struct header *header)
{
    struct string *str = string_create(serv.ip, strlen(serv.ip));
    string_concat_str(str, ":", 1);
    string_concat_str(str, serv.port, strlen(serv.port));
    if (!string_compare_n_str(header->host, str->data, header->host->size))
    {
        string_destroy(str);
        return 1;
    }

    string_destroy(str);

    if (!string_compare_n_str(header->host, serv.server_name->data,
                              header->host->size))
    {
        return 1;
    }

    if (!string_compare_n_str(header->host, serv.ip, header->host->size))
        return 1;

    return 0;
}
static int do_i_double_CRLF(struct string *str)
{
    for (size_t i = 0; i < str->size; i++)
        if (string_strstr(str, "\r\n\r\n"))
            return 1;

    return 0;
}

struct string *read_request(int cfd)
{
    char buffer[BUFFER_SIZE];
    reset_buffer(buffer);
    struct string *line = string_create("", 0);
    int read_size;
    while (!do_i_double_CRLF(line)
           && (read_size = recv(cfd, buffer, 1, 0)) != 0)
    {
        if (read_size == -1)
        {
            string_destroy(line);
            return NULL;
        }

        line = string_concat_str(line, buffer, read_size);
        reset_buffer(buffer);
        read_size = 0;
    }
    if (string_strstr(line, "Content-Length:"))
    {
        int size = how_many_length(line);
        if (size <= 0)
            return line;
        char *content = calloc(size + 1, sizeof(char));
        do
        {
            read_size = recv(cfd, content, size, 0);
            if (read_size == 0)
                break;
            if (read_size == -1)
            {
                string_destroy(line);
                free(content);
                return NULL;
            }

            line = string_concat_str(line, buffer, read_size);
            size -= read_size;
        } while (size > 0);
        free(content);
    }

    return line;
}
static void put_length(size_t len, struct string *request)
{
    char content_length[] = "Content-Length:";
    char val[BUFFER_SIZE];
    sprintf(val, "%s %ld\r\n", content_length, len);
    string_concat_str(request, val, strlen(val));
}
static void send_head(int cfd, int len, struct header *header)
{
    char response[] = "HTTP/1.1 200 OK\r\n";
    char closed[] = "Connection: close\r\n\r\n";
    struct string *anws = string_create(response, strlen(response));
    put_time(anws, 1);
    put_length(len, anws);
    string_concat_str(anws, closed, strlen(closed));
    if (header->method == HEAD)
        send(cfd, anws->data, anws->size, 0);
    else
        send(cfd, anws->data, anws->size, MSG_MORE);
    string_destroy(anws);
}

static void send_error(int cfd, int flag)
{
    char *response = NULL;
    switch (flag)
    {
    case FB:
        response = "HTTP/1.1 403 Forbidden\r\n";
        break;
    case NF:
        response = "HTTP/1.1 404 Not Found\r\n";
        break;

    case MNA:
        response = "HTTP/1.1 405 Method Not Allowed\r\n";
        break;
    case BR:
        response = "HTTP/1.1 400 Bad Request\r\n";
        break;
    case OK:
        exit(400);
        break;
    default:
        response = "HTTP/1.1 505 HTTP Version Not Supported\r\n";
    }
    char closed[] = "Connection: close\r\n\r\n";
    struct string *anws = string_create(response, strlen(response));
    put_time(anws, 1);
    string_concat_str(anws, closed, strlen(closed));
    send(cfd, anws->data, anws->size, 0);
    string_destroy(anws);
}

static int which_method(struct header *header, struct string *request)
{
    struct tmp
    {
        enum Method method;
        char *method_str;
    };
    static struct tmp method[] = {
        { GET, S_GET },
        { HEAD, S_HEAD },
    };
    for (size_t i = 0; i < sizeof(method) / sizeof(*method); i++)
    {
        if (!string_compare_n_str(request, method[i].method_str, request->size))
        {
            header->method = method[i].method;
            return 1;
        }
    }
    header->method = UNSUPPORTED;
    return 1;
}

void print_header(struct header *header)
{
    struct tmp
    {
        enum Method method;
        char *method_str;
    };
    static struct tmp method[] = {
        { GET, S_GET },
        { HEAD, S_HEAD },
    };
    for (size_t i = 0; i < sizeof(method) / sizeof(*method); i++)
    {
        if (header->method == method[i].method)
            printf("METHOD = %s\n", method[i].method_str);
    }
    printf("TARGET = %s\n", header->target->data);
    printf("HOST = %s\n", header->host->data);
    printf("CONTENT-LENGTH = %i\n", header->content_length);
}

static int get_CRLF(char *str, size_t max)
{
    size_t len = 0;
    char *copy = str;
    while (len < max && fnmatch("\r\n*", copy, 0))
    {
        len++;
        copy++;
    }
    if (len >= max)
        return -1;
    str[len] = '\0';
    return len;
}

static size_t which_version(struct header *header, char *request, size_t max)
{
    int len = get_CRLF(request, max);
    if (len == -1)
        return 0;
    if (!fnmatch("HTTP/?.?", request, 0))
    {
        if (!strcmp("HTTP/1.1", request))
        {
            header->good_version = 1;
            return ++len;
        }
        header->good_version = 0;
        return len;
    }
    return -1;
}

static struct string *get_method_target_version(struct header *header,
                                                struct string *request)
{
    size_t total = 0;
    int len = 0;
    if (request == NULL)
        return NULL;
    char *copy = request->data;
    if ((len = string_strcspn(request, " ", len)) == -1)
        return NULL;

    struct string *str = string_create(copy, len);
    if (!which_method(header, str))
        return NULL;
    string_destroy(str);
    copy += ++len;

    while (*copy == ' ')
    {
        copy++;
        len++;
    }

    total += len;
    if ((len = string_strcspn(request, " ", len)) == -1)
    {
        return NULL;
    }
    str = string_create(copy, len);
    header->target = str;
    copy += ++len;

    while (*copy == ' ')
    {
        copy++;
        len++;
    }

    total += len;

    if ((len = which_version(header, copy, request->size)) == -1)
    {
        return NULL;
    }
    copy += ++len;
    total += len;
    str = string_create(copy, request->size - total);
    return str;
}

static void get_host(struct header *header, char *request, int max)
{
    size_t len = get_CRLF(request, max);
    request += 6;
    header->host = string_create(request, len - 6);
    len -= 6;
    while (string_strcspn(header->host, " \t", 0) != -1)
    {
        struct string *tmp = string_create(++request, --len);
        string_destroy(header->host);
        header->host = tmp;
    }
}

static struct string *get_host_length(struct header *header,
                                      struct string *request)
{
    int len = string_strstr(request, "\r\n\r\n");
    int hosted = 0;
    int lengthed = 0;
    for (size_t i = 0; i < request->size; i++)
    {
        if (!string_compare_n_start_str(request, "Host: ", strlen("Host: "), i))
        {
            char *copy = request->data + i;

            if (!hosted)
            {
                get_host(header, copy, request->size - i);
                hosted = 1;
            }
            else
            {
                string_destroy(header->host);
                header->host = NULL;
            }
        }
        if (!string_compare_n_start_str(
                request, "Content-Length: ", strlen("Content-Length: "), i))
        {
            if (!lengthed)
            {
                header->content_length = how_many_length(request);
                if (header->content_length == -1)
                    header->content_length = -2;
                hosted = 1;
            }
            else
            {
                header->content_length = -2;
            }
        }
    }
    int diff = request->size - len;
    if (header->content_length != -1 && diff != header->content_length)
        header->content_length = -2;

    return request;
}

static void send_get(int cfd, struct header *header, struct server_config serv,
                     struct config *config)
{
    struct stat path_stat;
    struct string *path = string_create(serv.root_dir, strlen(serv.root_dir));

    if (header->target->size == 1 && header->target->data[0] == '/')
    {
        string_concat_str(path, header->target->data, header->target->size);
        string_concat_str(path, serv.default_file, strlen(serv.default_file));
    }
    else
        string_concat_str(path, header->target->data, header->target->size);

    if (stat(path->data, &path_stat) != 0)
    {
        string_destroy(path);
        logger_ok_recv(config, serv, header);
        send_error(cfd, NF);
        logger_send(config, NF, serv, header);
        return;
    }

    if (S_ISDIR(path_stat.st_mode))
    {
        string_concat_str(path, "/", 1);
        string_concat_str(path, serv.default_file, strlen(serv.default_file));
    }

    if (stat(path->data, &path_stat) != 0)
    {
        string_destroy(path);
        logger_ok_recv(config, serv, header);
        logger_send(config, NF, serv, header);
        send_error(cfd, NF);
        return;
    }

    int fd = open(path->data, O_RDONLY);
    string_destroy(path);
    if (fd == -1)
    {
        logger_ok_recv(config, serv, header);
        logger_send(config, FB, serv, header);
        send_error(cfd, FB);
        return;
    }
    logger_ok_recv(config, serv, header);
    logger_send(config, OK, serv, header);
    send_head(cfd, path_stat.st_size, header);

    if (header->method != HEAD && connected)
        sendfile(cfd, fd, 0, path_stat.st_size);

    close(fd);
}

static void respond(struct header *header, int cfd, struct server_config serv,
                    struct config *config)
{
    if (!header->good_version)
    {
        logger_ok_recv(config, serv, header);
        logger_send(config, VNS, serv, header);
        send_error(cfd, VNS);
        return;
    }
    if (!check_host(serv, header))
    {
        logger_error_recv(config, BR, serv);
        logger_send(config, BR, serv, header);
        send_error(cfd, BR);
        return;
    }

    switch (header->method)
    {
    case HEAD:
    case GET:
        send_get(cfd, header, serv, config);
        return;
    default:
        logger_ok_recv(config, serv, header);
        logger_send(config, MNA, serv, header);
        send_error(cfd, MNA);
    }
}
static struct string *cut_uri(struct string *uri,
                              struct server_config server_config)
{
    for (size_t i = 0; i < uri->size; i++)
    {
        if (!string_compare_n_start_str(uri, server_config.server_name->data,
                                        server_config.server_name->size, i))
        {
            char *copy = uri->data;
            copy += i + server_config.server_name->size;
            struct string *target = string_create(
                copy, uri->size - i - server_config.server_name->size);
            string_destroy(uri);
            return target;
        }
    }
    return NULL;
}
static struct string *decoding(struct server_config server_config,
                               struct string *uri)
{
    if (string_strstr(uri, server_config.server_name->data))
        uri = cut_uri(uri, server_config);
    if (uri->data[0] != '/')
    {
        string_destroy(uri);
        return NULL;
    }
    return UrlDecoding(uri);
}
void parse_request(int cfd, struct server_config serv, struct config *config,
                   struct string *request)
{
    connected = 1;
    struct sigaction sig;
    sig.sa_flags = 0;
    sig.sa_handler = &sigpipe_handler;
    if (sigemptyset(&sig.sa_mask) < 0)
        return;
    if (sigaction(SIGPIPE, &sig, NULL) < 0)
        return;
    struct header *header = calloc(1, sizeof(struct header));
    if (header == NULL)
        return;
    header->content_length = -1;
    header->method = UNSUPPORTED;

    if ((request = get_method_target_version(header, request)) == NULL)
    {
        if (header->target == NULL)
        {
            logger_error_recv(config, BR, serv);
            logger_send(config, BR, serv, header);
            send_error(cfd, BR);
        }
        else if (header->method == UNSUPPORTED)
        {
            logger_error_recv(config, MNA, serv);
            logger_send(config, MNA, serv, header);
            send_error(cfd, MNA);
        }
        else
        {
            logger_error_recv(config, BR, serv);
            logger_send(config, BR, serv, header);
            send_error(cfd, BR);
        }
        destroy_header(header);
        return;
    }
    header->target = decoding(serv, header->target);
    if (header->target != NULL)
        get_host_length(header, request);
    if (header->host == NULL || header->content_length == -2)
    {
        logger_error_recv(config, BR, serv);
        logger_send(config, BR, serv, header);
        send_error(cfd, BR);

        string_destroy(request);
        destroy_header(header);
        return;
    }
    respond(header, cfd, serv, config);
    string_destroy(request);
    destroy_header(header);
}

void destroy_header(struct header *header)
{
    string_destroy(header->host);
    string_destroy(header->target);
    free(header);
}
