#ifndef _SERVER_H
#define _SERVER_H
#define _GNU_SOURCE
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>

typedef enum _Method {
    GET,
    HEAD,
    POST,
    UNALLOWED
} Method;

int writeFileToSocket(int, int);

int api_handler(int, Method, char *, size_t);

#endif