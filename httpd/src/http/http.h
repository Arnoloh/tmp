#ifndef HTTP_H
#define HTTP_H

#include <fcntl.h>
#include <fnmatch.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "../config/config.h"
#include "../logger/logger.h"
#include "../utils/utils/utils.h"

void parse_request(int cfd, struct server_config serv, struct config *config,
                   struct string *request);

int how_many_length(struct string *str);

#endif /* HTTP_H */
