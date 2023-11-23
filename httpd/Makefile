export CC = gcc
export CPPFLAGS = -Isrc
export CFLAGS = -std=c99 -Werror -Wall -Wextra -Wvla -pedantic 


RED=\033[1;31m
NC=\033[0m


BINARY=httpd

all: $(BINARY)

$(BINARY):
	$(MAKE) -C src
	mv src/$(BINARY) .


check: all
	./tests/run_tests.sh

run: clean all
	echo -e "$(RED)Running Server$(NC)"
	./httpd server.conf

clean:
	$(MAKE) clean -C src/ 
	$(RM) httpd

.PHONY: all
