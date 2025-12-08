#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>


#define PORT 5001
#define MAX_WORLD_LEN 15
#define MAX_ATTEMPTS 6
#define BUF_SIZE 1024
