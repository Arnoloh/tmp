CC = gcc -g

# Pre-processor flags
DEBUG = -g
# Main compilation flags
CFLAGS = -Wall -Wextra  -Werror -Wvla -std=c99 
# Linker flags
LDFLAGS =

SRC = epoll.c
OBJ = ${SRC:.c=.o}


all: epoll

epoll: ${OBJ} 
	$(CC) $^ -o epoll

run: epoll
	./epoll ~/pipe
.PHONY:all epoll

clean:
	${RM} ${OBJ} ${NAME} epoll
