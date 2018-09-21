#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <signal.h>
#include <netdb.h>



#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))
#define HERR(source) (fprintf(stderr,"%s(%d) at %s:%d\n",source,h_errno,__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))

#define MESSAGE_SIZE 1

#define TIMEOUT 15

volatile sig_atomic_t last_signal=0 ;

void sigalrm_handler(int sig) {
	last_signal=sig;
}

int sethandler( void (*f)(int), int sigNo) {
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = f;
	if (-1==sigaction(sigNo, &act, NULL))
		return -1;
	return 0;
}

int make_socket(void){
	int sock;
	sock = socket(PF_INET,SOCK_DGRAM,0);
	if(sock < 0) ERR("socket");
	return sock;
}

struct sockaddr_in make_address(char *address, uint16_t port){
	struct sockaddr_in addr;
	struct hostent *hostinfo;
	addr.sin_family = AF_INET;
	addr.sin_port = htons (port);
	hostinfo = gethostbyname(address);
	if(hostinfo == NULL)HERR("gethostbyname");
	addr.sin_addr = *(struct in_addr*) hostinfo->h_addr;
	return addr;
}

void usage(char * name){
	fprintf(stderr,"USAGE: %s domain port sign \n",name);
}

void doClient(int fd, int16_t *sign, struct sockaddr_in *addr) 
{
		if(TEMP_FAILURE_RETRY(sendto(fd, sign,sizeof(int16_t),0,addr,sizeof(*addr)))<0)
		ERR("sendto:");
	alarm(TIMEOUT);
	
	int16_t *message = malloc(2 * sizeof(int16_t));

	while(recv(fd,(char *)message,2 * sizeof(int16_t),0)<0){
		if(EINTR!=errno)ERR("recv:");
		if(SIGALRM==last_signal) break;
	}
	printf("Message received from server %c%c\n", ntohs(message[0]), ntohs(message[1]));
	if(ntohs(message[0]) == 'O' && ntohs(message[1])=='K')
	{
		printf("Connected to server\n");
	}
}

int main(int argc, char** argv) {
	int fd;
	struct sockaddr_in addr;
	int16_t sign;
	int16_t deny=-1;
	deny=htons(deny);
	if(argc!=4) {
		usage(argv[0]);
		return EXIT_FAILURE;
	}
	if(sethandler(SIG_IGN,SIGPIPE)) ERR("Seting SIGPIPE:");
	if(sethandler(sigalrm_handler,SIGALRM)) ERR("Seting SIGALRM:");
	fd = make_socket();
	addr=make_address(argv[1],atoi(argv[2]));
	sign=htons(atoi( argv[3]));
	doClient(fd, &sign, &addr);
	/*
	 * Broken PIPE is treated as critical error here
	 */

	
	if(last_signal==SIGALRM)printf("Timeout\n");

	if(TEMP_FAILURE_RETRY(close(fd))<0)ERR("close");
	return EXIT_SUCCESS;
}
