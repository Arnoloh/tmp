#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

#include "../config/config.h"
#include "../http/http.h"
#include "../utils/utils/utils.h"

void greatefull_shutdown(struct server_config server, struct config *config);
int give_correct_fd(struct config *config);
void close_log(struct server_config serv, struct config *config);
void logger_error_recv(struct config *config, int flag,
                       struct server_config server);

void logger_ok_recv(struct config *config, struct server_config server,
                    struct header *header);

void logger_send(struct config *config, int flag, struct server_config server,
                 struct header *header);

#endif /* LOGGER_H */
