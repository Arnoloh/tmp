#define _POSIX_C_SOURCE 200809L
#include "server.h"

static int run = 1;

static struct config *config = NULL;
#define MAX_EVENTS 5
static struct reading *all_ptr = NULL;
struct reading
{
    struct reading *next;
    int fd;
    struct server_config serv;
    struct config *config;
    struct string *data;
    int body;
    int still_reading;
};
static char *path = NULL;

void set_path(char *path_tmp)
{
    path = path_tmp;
}
struct server
{
    int cfd;
    struct server_config serv;
    struct config *config;
};
static int global_sfd = 0;
static int multiserver(int sfd);

void reload_config(void)
{
    if (config->log)
        close_log(config->servers[0], config);
    struct config *new_config = parse_configuration(path);

    config_destroy(config);
    config = new_config;
    if (config->log_file == NULL)
        config->log_file = strdup("HTTPd.log");
    if (config->log)
        config->servers[0].log_fd = give_correct_fd(config);
}

void update_run(int sig)
{
    if (sig == SIGINT)
        run = 0;
    if (sig == SIGUSR1)
        reload_config();
}

static struct addrinfo *create_hints(struct server_config server)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *result;
    int err = getaddrinfo(server.ip, server.port, &hints, &result);
    if (err != 0)
    {
        if (server.log_fd != -1)
            write(server.log_fd, "getaddrinfo\n", strlen("getaddrinfo\n"));
        return NULL;
    }
    return result;
}

static int init_server(struct server_config server)
{
    struct addrinfo *result = create_hints(server);
    if (result == NULL)
        return -1;
    int sfd;
    int optval = 1;
    int err;
    struct addrinfo *p = NULL;
    const void *opt_void = &optval;
    for (p = result; p != NULL; p = p->ai_next)
    {
        sfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

        if (sfd == -1)
            continue;
        setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, opt_void, sizeof(int));
        err = bind(sfd, p->ai_addr, p->ai_addrlen);

        if (err == 0)
            break;
        else
            close(sfd);
    }
    freeaddrinfo(result);
    if (p == NULL)
    {
        if (server.log_fd != -1)
            write(server.log_fd, "socket error\n", strlen("socket error\n"));
        return -1;
    }
    return sfd;
}

void worker(void *arg)
{
    struct server *serv = arg;
    int cfd = serv->cfd;
    close(cfd);
}

static char *get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen)
{
    const void *tmp = sa;
    const struct sockaddr_in *sin = tmp;
    switch (sa->sa_family)
    {
    case AF_INET:
        inet_ntop(AF_INET, &sin->sin_addr, s, maxlen);
        break;

    default:
        strncpy(s, "Unknown AF", maxlen);
        return NULL;
    }

    return s;
}

static struct reading *init_reading(int fd)
{
    struct reading *reader = calloc(1, sizeof(struct reading));
    reader->next = all_ptr;
    all_ptr = reader;
    reader->fd = fd;
    reader->data = string_create("", 0);
    reader->config = config;
    reader->still_reading = 1;
    return reader;
}
static void free_reading(struct reading *reader)
{
    if (all_ptr == reader)
        all_ptr = reader->next;

    else
    {
        struct reading *read = all_ptr;
        while (read != NULL && read->next != reader)
            read = read->next;
        read->next = reader->next;
    }
    close(reader->fd);
    string_destroy(reader->data);
    free(reader);
}
static void update_to_write(struct reading *reader, int epoll_fd)
{
    struct epoll_event event;
    event.events = EPOLLOUT;
    event.data.ptr = reader;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, reader->fd, &event))
        free_reading(reader);
}

static int handle_client_connected(struct reading *reader, int epoll_fd)
{
    char buffer[256];
    int read_size = 0;
    if (reader->body != 0)
    {
        while (reader->still_reading
               && (read_size = recv(reader->fd, buffer, 1, MSG_DONTWAIT)) != 0)
        {
            if (read_size == -1)
                break;
            string_concat_str(reader->data, buffer, read_size);
            reader->body -= read_size;
            if (reader->body <= 0)
                reader->still_reading = 0;
        }
        if (read_size == 0)
            return 0;
        if (reader->body > 0)
            return 1;
    }
    else
    {
        while (reader->still_reading
               && (read_size = recv(reader->fd, buffer, 1, MSG_DONTWAIT)) != 0)
        {
            if (read_size == -1)
                break;
            string_concat_str(reader->data, buffer, read_size);
            if (string_strstr(reader->data, "\r\n\r\n") != 0)
                reader->still_reading = 0;
        }

        if (reader->still_reading == 0)
        {
            if (string_strstr(reader->data, "Content-Length: "))
            {
                int size = how_many_length(reader->data);
                if (size > 0)
                {
                    reader->still_reading = 1;
                    reader->body = size;
                    return handle_client_connected(reader, epoll_fd);
                }
            }
        }
    }
    if (read_size == 0)
        return 0;

    if (reader->still_reading == 0)
    {
        update_to_write(reader, epoll_fd);
    }
    return 1;
}
static void accept_server(int epoll_fd, int sfd)
{
    struct sockaddr client_addr;
    socklen_t addrlen = sizeof(struct sockaddr);

    int cfd = accept(sfd, &client_addr, &addrlen);
    struct epoll_event tmp;
    tmp.events = EPOLLET | EPOLLIN;
    tmp.data.ptr = init_reading(cfd);
    struct reading *reader = tmp.data.ptr;
    reader->serv = config->servers[0];
    get_ip_str(&client_addr, reader->serv.ip_client, INET_ADDRSTRLEN);
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, cfd, &tmp))
    {
        run = 0;
    }
}

static void to_nonblocking(int sfd)
{
    int flags = fcntl(sfd, F_GETFL, 0);
    fcntl(sfd, F_SETFL, flags | O_NONBLOCK);
}
static void destroy_all(void)
{
    struct reading *reader = all_ptr;
    while (reader != NULL)
    {
        struct reading *tmp = reader->next;
        close(reader->fd);
        string_destroy(reader->data);
        free(reader);
        reader = tmp;
    }
}
static int multiserver(int sfd)
{
    struct epoll_event event;
    event.events = EPOLLIN;
    struct reading *serv_reading = init_reading(sfd);
    event.data.ptr = serv_reading;

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        return 1;
    }
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sfd, &event))
    {
        close(epoll_fd);
        close(sfd);
        return 1;
    }
    struct epoll_event events[MAX_EVENTS];

    while (run)
    {
        int wait = epoll_wait(epoll_fd, events, MAX_EVENTS, 30000);
        for (int i = 0; i < wait; i++)
        {
            struct reading *actual = events[i].data.ptr;
            if (actual->fd == sfd)
            {
                accept_server(epoll_fd, sfd);
            }
            else if (actual->still_reading != 1)
            {
                parse_request(actual->fd, actual->serv, config, actual->data);

                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, actual->fd, &event);
                free_reading(actual);
            }
            else
            {
                if (!handle_client_connected(actual, epoll_fd))
                {
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, actual->fd, &event);
                    free_reading(actual);
                }
            }
        }
    }

    if (config->log == 1)
    {
        greatefull_shutdown(config->servers[0], config);
        close_log(config->servers[0], config);
    }

    close(epoll_fd);
    destroy_all();
    return 0;
}

static int server(void)
{
    if (config->log == 1)
    {
        config->servers[0].log_fd = give_correct_fd(config);
    }

    int sfd = init_server(config->servers[0]);
    if (sfd == -1)
        return -1;
    int err = listen(sfd, 5);
    if (err == -1)
    {
        close(sfd);
        if (config->servers[0].log_fd != -1)
            write(config->servers[0].log_fd, "error listen on serv\n",
                  strlen("error listen on serv\n"));
        return -1;
    }
    to_nonblocking(sfd);
    global_sfd = sfd;

    return multiserver(sfd);
}

void lunch_serverHTTPD(struct config *config_tmp)
{
    config = config_tmp;
    server();
    config_destroy(config);
}
