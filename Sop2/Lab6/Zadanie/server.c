#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <signal.h>
#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>
#define PORT 2000
#define MAXCLIENT 5
#define MAXBUF 10
#define ERR(source) (perror(source),                                 \
					 fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), \
					 exit(EXIT_FAILURE))

volatile sig_atomic_t do_work = 1;
struct client_t
{
	struct sockaddr_in addr;
	int status;
	int num;
} typedef client;

void sigint_handler(int sig)
{
	do_work = 0;
	printf("SIGINT received and handled\n");
}

struct arg
{
	int fd;
	int wait;
	struct sockaddr_in addr;
	sem_t *semaphore;
	unsigned int seed;
	int number;
} typedef arg_t;

void usage(char *name)
{
	fprintf(stderr, "USAGE: %s   \n", name);
	fprintf(stderr, "No arguments\n");
	exit(EXIT_FAILURE);
}

int sethandler(void (*f)(int), int sigNo)
{
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = f;
	if (-1 == sigaction(sigNo, &act, NULL))
		return -1;
	return 0;
}
int make_socket(int domain, int type)
{
	int sock;
	sock = socket(domain, type, 0);
	if (sock < 0)
		ERR("socket");
	return sock;
}
int bind_inet_socket(uint16_t port, int type)
{
	struct sockaddr_in addr;
	int socketfd, t = 1;
	socketfd = make_socket(PF_INET, type);
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &t, sizeof(t)))
		ERR("setsockopt");
	if (bind(socketfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		ERR("bind");
	return socketfd;
}

void *thread_work(void *a)
{
	arg_t *args = (arg_t *)a;
	if (args->wait)
		sem_wait(args->semaphore);
	char buf[MAXBUF];
	strcpy(buf, "OK");
	if (TEMP_FAILURE_RETRY(sendto(args->fd, (char *)&buf, MAXBUF, 0, &(args->addr), sizeof(args->addr))) < 0)
		ERR("sendto");
	printf("Client <%d> is accepted\n", args->number);
	sleep(8);

	// volatile sig_atomic_t do_thread_work = 1;
	while (do_work)
	{
	}
	printf("Thread stopped working\n");
	if (sem_post(args->semaphore) == -1)
		ERR("sem_post");
	return NULL;
}

void server_work(int fd, client *clients, pthread_t *threads, int *currentClient)
{
	srand(time(NULL));
	pthread_t thread;
	arg_t *args;
	char buf[MAXBUF];
	struct sockaddr_in addr;
	sem_t semaphore;
	socklen_t size = sizeof(struct sockaddr_in);
	if (sem_init(&semaphore, 0, MAXCLIENT) != 0)
		ERR("sem_init");
	while (do_work)
	{
		if (recvfrom(fd, buf, sizeof(int16_t), 0, &addr, &size) < 0)
		{
			if (errno == EINTR)
				continue;
			else
				ERR("recvfrom");
		}
		if (TEMP_FAILURE_RETRY(sem_wait(&semaphore)) == -1)
		{
			printf("-1 in sem_wait\n");
			switch (errno)
			{
			case EAGAIN:
				if ((args = malloc(sizeof(arg_t))) == NULL)
					ERR("malloc");
				args->fd = fd;
				args->addr = addr;
				args->semaphore = &semaphore;
				args->seed = rand();
				args->wait = 1;
				args->number = buf[0];
				if (pthread_create(&threads[0], NULL, thread_work, (void *)args) != 0)
					ERR("pthread_create");
				printf("Current client value %d", *currentClient);
				// *currentClient = (*currentClient) + 1;
				if (pthread_detach(&threads[0]) != 0)
					ERR("pthread_detach");
				break;
			case EINTR:
				continue;
			}
			ERR("sem_wait");
		}

		if ((args = malloc(sizeof(arg_t))) == NULL)
			ERR("malloc");
		args->fd = fd;
		args->addr = addr;
		args->semaphore = &semaphore;
		args->seed = rand();
		args->wait = 0;

		args->number = buf[0];
		if (pthread_create(&threads[0], NULL, thread_work, (void *)args) != 0)
			ERR("pthread_create");
		// *currentClient = (*currentClient) + 1;
		if (pthread_detach(&threads[0]) != 0)
			ERR("pthread_detach");
	}
}

int main(int argc, char **argv)
{
	if (argc != 1)
		usage(argv[0]);
	int fd = bind_inet_socket(PORT, SOCK_DGRAM);
	client clients[MAXCLIENT];
	for (int i = 0; i < MAXCLIENT; i++)
		clients[i].status = -1;
	pthread_t threads[MAXCLIENT];
	for (int i = 0; i < MAXCLIENT; i++)
	{
		threads[i] = -1;
	}

	int *currentClient = malloc(sizeof(int));
	*currentClient = 0;
	sethandler(sigint_handler, SIGINT);
	server_work(fd, clients, &threads, currentClient);
	for (int i = 0; i < MAXCLIENT; i++)
			if (pthread_join(threads[0], NULL) != 0)
				ERR("pthread_join");

	if (TEMP_FAILURE_RETRY(close(fd)) < 0)
		ERR("close");
	return EXIT_SUCCESS;
}
