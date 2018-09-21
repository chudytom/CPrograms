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
#define MAXCLIENT 3
#define MAXBUF 10
#define ERR(source) (perror(source),\
                     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     exit(EXIT_FAILURE))

struct client_t{
	struct sockaddr_in addr;
	int status;
	int num;
} typedef client;

struct arg{
	int fd;
	int wait;
	struct sockaddr_in addr;
	sem_t *semaphore;
	unsigned int seed;
} typedef arg_t;

volatile sig_atomic_t do_work=1 ;

void usage(char * name){
        fprintf(stderr,"USAGE: %s domain port  \n",name);
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

ssize_t bulk_write(int fd, char *buf, size_t count){
        int c;
        size_t len=0;
        do{
                c=TEMP_FAILURE_RETRY(write(fd,buf,count));
                if(c<0) return c;
                buf+=c;
                len+=c;
                count-=c;
        }while(count>0);
        return len ;
}

void* thread_work(void *a)
{
	arg_t *args=(arg_t*)a;
	if(args->wait) sem_wait(args->semaphore);
	int16_t num=rand_r(&args->seed)%10+20;
	if(TEMP_FAILURE_RETRY(sendto(args->fd,(char *)&num,sizeof(int16_t),0,&(args->addr),sizeof(args->addr)))<0) ERR("sendto");
	printf("Client with number <%d> is accepted\n",num);
	sleep(5);
	if (sem_post(args->semaphore) == -1) ERR("sem_post");
	return NULL;
}

void server_work(int fd,client *clients)
{
	srand(time(NULL));
	pthread_t thread;
	arg_t *args;
	char buf[MAXBUF];
	struct sockaddr_in addr;
	sem_t semaphore;
	socklen_t size=sizeof(struct sockaddr_in);
	if(sem_init(&semaphore,0,MAXCLIENT)!=0) ERR("sem_init");
	while(do_work)
	{
		if(recvfrom(fd,buf,MAXBUF,0,&addr,&size)<0)
		{
			if(errno==EINTR) continue;
			else ERR("recvfrom");
		}
		if (TEMP_FAILURE_RETRY(sem_wait(&semaphore)) == -1)
		{
			switch(errno)
			{
				case EAGAIN:
					if((args=malloc(sizeof(arg_t)))==NULL) ERR("malloc");
					args->fd=fd;
					args->addr=addr;
					args->semaphore=&semaphore;
					args->seed=rand();
					args->wait=1;
					if(pthread_create(&thread,NULL,thread_work,(void*)args)!=0) ERR("pthread_create");
					if(pthread_detach(thread)!=0) ERR("pthread_detach");
					break;
                case EINTR:     continue;
            }
            ERR("sem_wait");
        }
        if(strcmp(buf,"BEGIN")==0)
        {
			if((args=malloc(sizeof(arg_t)))==NULL) ERR("malloc");
			args->fd=fd;
			args->addr=addr;
			args->semaphore=&semaphore;
			args->seed=rand();
			args->wait=0;
			if(pthread_create(&thread,NULL,thread_work,(void*)args)!=0) ERR("pthread_create");
			if(pthread_detach(thread)!=0) ERR("pthread_detach");
		}
	}
}
                     
int main(int argc, char **argv)
{
	int fd=bind_inet_socket(PORT,SOCK_DGRAM);
	client clients[MAXCLIENT];
	for(int i=0;i<MAXCLIENT;i++) clients[i].status=-1;
	server_work(fd,clients);
	if(TEMP_FAILURE_RETRY(close(fd))<0) ERR("close");
	return EXIT_SUCCESS;
}
