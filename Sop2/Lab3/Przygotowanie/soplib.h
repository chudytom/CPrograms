#ifndef _SOPLIB_H
#define _SOPLIB_H

#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <signal.h>
#include <netdb.h>
#include <fcntl.h>
#include <time.h>

char* concat(const char* s1, const char* s2);
char* convertToCapital(const char* text);
void basicUsage(char* pname);
void usageVerbose();
ssize_t bulk_read(int fd, char *buff, size_t count);
ssize_t bulk_write(int fd, char *buff, size_t count);
int sethandler(void (*f)(int), int sigNo);
int make_socket(char *name, struct sockaddr_un *addr);
int bind_socket(char *name, int waitingConnections);
int add_new_client(int sfd);
int connect_socket(char *name);
int connect_tcp_socket(char *name, char *port);
struct sockaddr_in make_address(char *address, char *port);
int bind_tcp_socket(uint16_t port, int waitingConnections);
int bind_local_socket(char *name, int waitingConnections);
int make_tcp_socket(int domain, int type);
int make_tcp_socket_client(void);


#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]));
#define ERR(source) (perror(source),\
					fprintf(stderr, "eror: %s:%d\n", __FILE__, __LINE__),\
					exit(EXIT_FAILURE))

#define BACKLOG 3
#define MAX_CLIENT 6
#define MESSAGE_SIZE 3

#endif