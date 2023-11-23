#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

void reset(char buffer[256])
{
    for (size_t i = 0; i < 256; i++)
        buffer[i] = 0;
}
int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "./epoll: Bad usage ./epoll <pipe_name>\n");
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1)
        return 1;

    struct epoll_event event;
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        return 1;
    }
    event.events = EPOLLET | EPOLLIN;
    event.data.fd = fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event))
    {
        close(epoll_fd);
        close(fd);
        return 1;
    }
    int running = 1;
    struct epoll_event events[1];
    char buffer[256] = { 0 };
    int read_size = 0;
    while (running)
    {
        int count = epoll_wait(epoll_fd, events, 1, 5000);
        for (int i = 0; i < count; i++)
        {
            read_size = read(events[i].data.fd, buffer, 256);
            if (read_size == -1)
                break;
            if (!strcmp(buffer, "quit"))
            {
                printf("quit\n");
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &event);
                running = 0;
            }
            else if (!strcmp(buffer, "ping"))
                printf("pong!\n");
            else if (!strcmp(buffer, "pong"))
                printf("ping!\n");
            else
                printf("Unknown: %s\n", buffer);
            reset(buffer);
        }
    }
    close(fd);
    close(epoll_fd);
    return 0;
}
