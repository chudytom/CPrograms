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
#define ERR(source) (perror(source),\
                     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     exit(EXIT_FAILURE))

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
struct sockaddr_in make_address(char *address, char *port){
        int ret;
        struct sockaddr_in addr;
        struct addrinfo *result;
        struct addrinfo hints = {};
        hints.ai_family = AF_INET;
        if((ret=getaddrinfo(address,port, &hints, &result))){
                fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
                exit(EXIT_FAILURE);
        }
        addr = *(struct sockaddr_in *)(result->ai_addr);
        freeaddrinfo(result);
        return addr;
}
void usage(char * name){
        fprintf(stderr,"USAGE: %s domain port time \n",name);
}
ssize_t bulk_read(int fd, char *buf, size_t count){
        int c;
        size_t len=0;
        do{
                c=TEMP_FAILURE_RETRY(read(fd,buf,count));
                if(c<0) return c;
                if(0==c) return len;
                buf+=c;
                len+=c;
                count-=c;
        }while(count>0);
        return len ;
}

void client_work(int fd,struct sockaddr_in addr)
{
	char buf[MAXBUF];
	int16_t number;
	strcpy(buf,"BEGIN");
	if(TEMP_FAILURE_RETRY(sendto(fd,buf,MAXBUF,0,&addr,sizeof(addr)))<0) ERR("sendto");
	printf("Sent BEGIN to server\n");
	while(recv(fd,(char*)&number,sizeof(int16_t),0)<0)
	{
		if(EINTR!=errno)ERR("recv:");
	}
	printf("Connected to server. Received number <%d>\n",number);
}
                     
int main(int argc,char **argv)
{
	int fd=make_socket();
	struct sockaddr_in addr=make_address(HOST,PORT);
	client_work(fd,addr);
	if(TEMP_FAILURE_RETRY(close(fd))<0)ERR("close");
	return EXIT_SUCCESS;
}
