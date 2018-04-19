#include "soplib.h"

char *concat(const char *s1, const char *s2)
{
    const size_t len1 = strlen(s1);
    const size_t len2 = strlen(s2);

    char *result = malloc(len1 + len2 + 1);

    memcpy(result, s1, len1);
    memcpy(result + len1, s2, len2 + 1);
    return result;
}

char *convertToCapital(const char *text)
{
    size_t len = NELEMS(text);
    char *result = malloc(len * sizeof(char));
    strcpy(result, text);
    for (int i = 0; i < len; i++)
    {
        if (result[i] >= 'a' && result[i] <= 'z')
            result[i] = result[i] + ('A' - 'a');
    }
    return result;
}

void basicUsage(char *pname)
{
    fprintf(stderr, "USAGE:%s name times>0\n", pname);
    exit(EXIT_FAILURE);
}

void usageVerbose(void)
{
    fprintf(stderr, "USAGE: signals n k p r\n");
    fprintf(stderr, "n - number of children\n");
    fprintf(stderr, "k - Interval before SIGUSR1\n");
    fprintf(stderr, "p - Interval before SIGUSR2\n");
    fprintf(stderr, "r - lifetime of child in cycles\n");
    exit(EXIT_FAILURE);
}

// #include <unistd.h> //optional
// #include <getopt.h>
// void readArguments()
// {
//     int x = 1;
//     char c = 'm';
//     while ((c = getopt(argc, argv, "t:n:")) != -1)
//     {
//         switch (c)
//         {
//         case 't':
//             x = atoi(optarg);
//             break;
//         case 'n':
//             for (int i = 0; i < x; i++)
//                 printf("Hello %s\n", optarg);
//             break;
//         default:
//             printf("Default\n");
//             usage(argv[0]);
//         }
//     }
//     if (argc > optind)
//         usage(argv[0]);
// }

ssize_t bulk_read(int fd, char *buff, size_t count)
{
    int c;
    size_t len = 0;
    do
    {
        c = TEMP_FAILURE_RETRY(read(fd, buff, count));
        if (c < 0)
            return c;
        if (0 == c)
            return len;
        buff += c; //Przesuwamy buffer o to co już mieliśmy
        len += c;

        count -= c;
    } while (count > 0);
    return len;
}

ssize_t bulk_write(int fd, char *buff, size_t count)
{
    int c;
    size_t len = 0;
    do
    {
        c = TEMP_FAILURE_RETRY(write(fd, buff, count));
        if (c < 0)
            return c;
        buff += c;
        len += c;
        count -= c;
    } while (count > 0);
    return len;
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

int make_socket(char *name, struct sockaddr_un *addr)
{
    int socketfd;
    if ((socketfd = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
        ERR("socket");
    memset(addr, 0, sizeof(struct sockaddr_un));
    addr->sun_family = AF_UNIX;
    strncpy(addr->sun_path, name, sizeof(addr->sun_path) - 1);
    return socketfd;
}

int bind_socket(char *name, int waitingConnections)
{
    struct sockaddr_un addr;
    int socketfd;
    if (unlink(name) < 0 && errno != ENOENT)
        ERR("unlink");
    socketfd = make_socket(name, &addr);
    if (bind(socketfd, (struct sockaddr *)&addr, SUN_LEN(&addr)) < 0)
        ERR("bind");
    if (listen(socketfd, waitingConnections))
        ERR("listen");
    return socketfd;
}

int add_new_client(int sfd)
{
    int nfd;
    if ((nfd = TEMP_FAILURE_RETRY(accept(sfd, NULL, NULL))) < 0)
    {
        if (EAGAIN == errno || EWOULDBLOCK == errno)
            return -1;
        ERR("accept");
    }
    return nfd;
}

int connect_socket(char *name)
{
    struct sockaddr_un addr;
    int socketfd;
    socketfd = make_socket(name, &addr);
    if (connect(socketfd, (struct sockaddr *)&addr, SUN_LEN(&addr)) < 0)
    {
        if (errno != EINTR)
            ERR("connect");
        else
        {
            fd_set wfds;
            int status;
            socklen_t size = sizeof(int);
            FD_ZERO(&wfds);
            FD_SET(socketfd, &wfds);
            if (TEMP_FAILURE_RETRY(select(socketfd + 1, NULL, &wfds, NULL, NULL)) < 0)
                ERR("select");
            if (getsockopt(socketfd, SOL_SOCKET, SO_ERROR, &status, &size) < 0)
                ERR("getsockopt");
            if(0!= status)
                ERR("connect");
        }
    }
    return socketfd;
}

int make_tcp_socket(int domain, int type)
{
    int sock;
    sock = socket(domain,type,0);
    if(sock < 0) ERR("socket");
    return sock;
}

int make_tcp_socket_client(void)
{
    int sock;
    sock = socket(PF_INET,SOCK_STREAM,0);
    if(sock < 0) ERR("socket");
    return sock;
}


int bind_local_socket(char *name, int waitingConnections){
    struct sockaddr_un addr;
    int socketfd;
    if(unlink(name) <0&&errno!=ENOENT) ERR("unlink");
    socketfd = make_tcp_socket(PF_UNIX,SOCK_STREAM);
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path,name,sizeof(addr.sun_path)-1);
    if(bind(socketfd,(struct sockaddr*) &addr,SUN_LEN(&addr)) < 0)  ERR("bind");
    if(listen(socketfd, waitingConnections) < 0) ERR("listen");
    return socketfd;
}

int bind_tcp_socket(uint16_t port, int waitingConnections){
    struct sockaddr_in addr;
    int socketfd,t=1;
    socketfd = make_tcp_socket(PF_INET,SOCK_STREAM);
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR,&t, sizeof(t))) ERR("setsockopt");
    if(bind(socketfd,(struct sockaddr*) &addr,sizeof(addr)) < 0)  ERR("bind");
    if(listen(socketfd, waitingConnections) < 0) ERR("listen");
    return socketfd;
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

int connect_tcp_socket(char *name, char *port){
        struct sockaddr_in addr;
        int socketfd;
        socketfd = make_tcp_socket_client();
        addr=make_address(name,port);
        if(connect(socketfd,(struct sockaddr*) &addr,sizeof(struct sockaddr_in)) < 0){
                if(errno!=EINTR) ERR("connect");
                else {
                        fd_set wfds ;
                        int status;
                        socklen_t size = sizeof(int);
                        FD_ZERO(&wfds);
                        FD_SET(socketfd, &wfds);
                        if(TEMP_FAILURE_RETRY(select(socketfd+1,NULL,&wfds,NULL,NULL))<0) ERR("select");
                        if(getsockopt(socketfd,SOL_SOCKET,SO_ERROR,&status,&size)<0) ERR("getsockopt");
                        if(0!=status) ERR("connect");
                }
        }
        return socketfd;
}

