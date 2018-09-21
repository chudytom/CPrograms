#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <signal.h>
#include <netdb.h>
#define HOST "localhost"
#define PORT "2000"
#define MAXBUF 10
#define ERR(source) (perror(source),                                 \
                     fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), \
                     exit(EXIT_FAILURE))

int sethandler(void (*f)(int), int sigNo)
{
        struct sigaction act;
        memset(&act, 0, sizeof(struct sigaction));
        act.sa_handler = f;
        if (-1 == sigaction(sigNo, &act, NULL))
                return -1;
        return 0;
}
int make_socket(void)
{
        int sock;
        sock = socket(PF_INET, SOCK_DGRAM, 0);
        if (sock < 0)
                ERR("socket");
        return sock;
}
struct sockaddr_in make_address(char *address, char *port)
{
        int ret;
        struct sockaddr_in addr;
        struct addrinfo *result;
        struct addrinfo hints = {};
        hints.ai_family = AF_INET;
        if ((ret = getaddrinfo(address, port, &hints, &result)))
        {
                fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
                exit(EXIT_FAILURE);
        }
        addr = *(struct sockaddr_in *)(result->ai_addr);
        freeaddrinfo(result);
        return addr;
}
void usage(char *name)
{
        fprintf(stderr, "USAGE: %s n \n", name);
        fprintf(stderr, "10<=n<20\n");
        exit(EXIT_FAILURE);
}

void client_work(int fd, struct sockaddr_in addr, int n)
{
        // int16_t number;
        char buf[MAXBUF];
        strcpy(buf, "BEGIN");
        if (TEMP_FAILURE_RETRY(sendto(fd, &n, MAXBUF, 0, &addr, sizeof(addr))) < 0)
                ERR("sendto");
        printf("Sent number %d to server\n", n);
        while (recv(fd, (char *)&buf, MAXBUF, 0) < 0)
        {
                if (EINTR != errno)
                        ERR("recv:");
        }
        if (strcpy(buf, "OK"))
        {
                printf("Connected to the server.\n");
        }
        else
        {
                printf("Not connected to server\n");
        }
        while(1);
}

int main(int argc, char **argv)
{
        if (argc != 2)
                usage(argv[0]);
        int n = atoi(argv[1]);
        if (n < 10 || n >= 20)
                usage(argv[1]);
        int fd = make_socket();
        struct sockaddr_in addr = make_address(HOST, PORT);
        client_work(fd, addr, n);
        if (TEMP_FAILURE_RETRY(close(fd)) < 0)
                ERR("close");
        return EXIT_SUCCESS;
}
