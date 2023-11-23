SRC_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

config/libconfig.a:
	$(MAKE) -C $(SRC_DIR)config

utils/libutils.a:
	$(MAKE) -C $(SRC_DIR)utils

server/libserver.a:
	$(MAKE) -C $(SRC_DIR)server

daemon/libdaemon.a:
	$(MAKE) -C $(SRC_DIR)daemon

handle_arg/libhandle_opt.a:
	$(MAKE) -C $(SRC_DIR)handle_arg

http/libhttp.a:
	$(MAKE) -C $(SRC_DIR)http

logger/liblogger.a:
	$(MAKE) -C $(SRC_DIR)logger
