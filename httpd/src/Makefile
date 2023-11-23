include flags.mk

OBJ = main.o

LIBS = config/libconfig.a utils/libutils.a
LIBS += server/libserver.a daemon/libdaemon.a handle_arg/libhandle_opt.a
LIBS += http/libhttp.a logger/liblogger.a


RED=\033[1;31m
NC=\033[0m



all: httpd

httpd: $(OBJ) $(LIBS)
	$(CC) $(LDFLAGS) -o $@ $(OBJ) -Wl,--start-group $(LIBS) -Wl,--end-group

check:
	$(MAKE) check -C config
	$(MAKE) check -C server
	$(MAKE) check -C daemon
	$(MAKE) check -C logger
	$(MAKE) check -C utils
	$(MAKE) check -C http
	$(MAKE) check -C handle_arg




run: httpd
	echo -e "$(RED)Running Server$(NC)"
	./httpd ../server.conf

include libs.mk

clean:
	$(MAKE) clean -C config
	$(MAKE) clean -C server
	$(MAKE) clean -C daemon
	$(MAKE) clean -C logger
	$(MAKE) clean -C utils
	$(MAKE) clean -C http
	$(MAKE) clean -C handle_arg


	$(RM) ${OBJ} httpd

.PHONY: all clean $(SUBDIRS) check
