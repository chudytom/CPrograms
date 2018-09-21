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
#include <pthread.h>
#include <semaphore.h>

#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))
#define FS_NUM 10 

#define MESSAGE_SIZE 2

volatile sig_atomic_t do_work=1 ;

void sigint_handler(int sig) {
	do_work=0;
}

int sethandler( void (*f)(int), int sigNo) {
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = f;
	if (-1==sigaction(sigNo, &act, NULL))
		return -1;
	return 0;
}

int make_socket(int domain, int type){
	int sock;
	sock = socket(domain,type,0);
	if(sock < 0) ERR("socket");
	return sock;
}

int bind_inet_socket(uint16_t port,int type){
	struct sockaddr_in addr;
	int socketfd,t=1;
	socketfd = make_socket(PF_INET,type);
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR,&t, sizeof(t))) ERR("setsockopt");
	if(bind(socketfd,(struct sockaddr*) &addr,sizeof(addr)) < 0)  ERR("bind");
	return socketfd;
}

void usage(char * name){
	fprintf(stderr,"USAGE: %s  port\n",name);
}

struct arguments {
	int fd;
	int16_t *sign;
	int16_t identifier;
	struct sockaddr_in addr;
	sem_t* semaphore;
};

void* communicateDgram(void *arg){
	struct arguments  *args= (struct arguments*) arg;
	fprintf(stderr,"Message to be sent %c%c\n",ntohs(args->sign[0]), ntohs(args->sign[1]));
	printf("Client %d is accepted\n", args->identifier);
	// for (tt = ntohs(args->sign); tt > 0; tt = sleep(tt));
	if(TEMP_FAILURE_RETRY(sendto(args->fd,(char *)((args->sign)),2 * sizeof(int16_t),0,&(args->addr),sizeof(args->addr)))<0&&errno!=EPIPE) ERR("sendto");
	if (sem_post(args->semaphore) == -1) ERR("sem_post");
	free(args);
	return NULL;
}

void doServer(int fd){
	int16_t identifier;
	int16_t deny=-1;
	deny=htons(deny);
	pthread_t thread;
	struct sockaddr_in addr;
	struct arguments *args;
	socklen_t size=sizeof(struct sockaddr_in);
	sem_t semaphore;
	if (sem_init(&semaphore, 0, FS_NUM) != 0)
		ERR("sem_init");
	while(do_work){
		if(recvfrom(fd,(char *)&identifier,sizeof(int16_t),0,&addr,&size)<0) {
			if(errno==EINTR) continue ;
			ERR("recvfrom:");
		}
		printf("Sign received from client %d\n", ntohs(identifier));

		if (TEMP_FAILURE_RETRY(sem_trywait(&semaphore)) == -1) {
			switch(errno){
			case EAGAIN: 	if(TEMP_FAILURE_RETRY(sendto(fd,(char *)&deny,sizeof(int16_t),0,&addr,sizeof(addr)))<0&&errno!=EPIPE) ERR("sendto");
			case EINTR: 	continue;
			}
			ERR("sem_wait");
		}

		if((args=(struct arguments*)malloc(sizeof(struct arguments)))==NULL) ERR("malloc:");
		args->sign = malloc(MESSAGE_SIZE * sizeof(int16_t));
		args->fd=fd;
		args->identifier = htons(identifier);
		printf("Idenftifier: %d\n", args->identifier);
		args->sign[0]= htons('O');
		args->sign[1]= htons('K');
		args->addr=addr;
		args->semaphore=&semaphore;
		if (pthread_create(&thread, NULL,communicateDgram, (void *)args) != 0) ERR("pthread_create");
		if (pthread_detach(thread) != 0) ERR("pthread_detach");

	}
}

int main(int argc, char** argv) {
	int fd;
	if(argc!=2) {
		usage(argv[0]);
		return EXIT_FAILURE;
	}
	if(sethandler(SIG_IGN,SIGPIPE)) ERR("Seting SIGPIPE:");
	if(sethandler(sigint_handler,SIGINT)) ERR("Seting SIGINT:");
	fd=bind_inet_socket(atoi(argv[1]),SOCK_DGRAM);
	doServer(fd);
	if(TEMP_FAILURE_RETRY(close(fd))<0)ERR("close");
	fprintf(stderr,"Server has terminated.\n");
	return EXIT_SUCCESS;
}
