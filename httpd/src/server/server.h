#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <err.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "../logger/logger.h"
#include "config/config.h"
#include "http/http.h"

void update_run(int sig);
void set_path(char *path_tmp);
void lunch_serverHTTPD(struct config *config);

#endif /* SERVER_H */
