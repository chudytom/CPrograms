#include "soplib.h"

#define ARGUMENTS_COUNT 3

volatile sig_atomic_t do_work = 1;

void sigint_handler(int sig)
{
    do_work = 0;
}

void usage(char *name)
{
    fprintf(stderr, "USAGE: %s tcpSocket port\n", name);
}

void prepare_request(char **argv, int32_t data[MESSAGE_SIZE])
{
    data[0] = htonl(atoi(argv[3]));
    data[1] = htonl(atoi(argv[4]));
    data[2] = htonl(0);
    data[3] = htonl((int32_t)(argv[5][0]));
    data[4] = htonl(1);
}

void resolveMessage(int32_t data[])
{
    char message = (char)ntohl(data[0]);
    printf("Character received from server %c\n", message);
    switch (message)
    {
    case 'F':
        do_work = 0;
        break;
    case 'N':
        break;
    case 'M':
        break;
    case 'H':
        break;
    default:
        printf("Unknown message received from server\n");
    }
}

void prepareStartMessage(int32_t *data)
{
    data[0] = htonl('N');
    data[1] = getpid();
    data[2] = 3;
}

void prepareNumberMessage(int32_t * data)
{
    data[0] = htonl('N');
    data[1] = getpid();
    srand(time(NULL));
    int randomNumber = rand();
    data[2] = randomNumber;
}

void doClient(int fd)
{
    int32_t *data = malloc(sizeof(int32_t[MESSAGE_SIZE]));
    prepareStartMessage(data);
    if (bulk_write(fd, (char *)data, sizeof(int32_t[MESSAGE_SIZE])) < 0)
        ERR("write:");
    while (do_work)
    {
        sleep(2);
        prepareNumberMessage(data);
        if(bulk_write(fd,(char *)data,sizeof(int32_t[MESSAGE_SIZE]))<0) ERR("write:");
        if (bulk_read(fd, (char *)data, sizeof(int32_t[MESSAGE_SIZE])) < (int)sizeof(int32_t[MESSAGE_SIZE]))
            printf("Message read %c %d %d\n", (char)htonl(data[0]), (char)htonl(data[1]), (char)htonl(data[2]));
            // printf("Something else");
        resolveMessage(data);
    }
}

int main(int argc, char **argv)
{
    int fd;
    // int32_t data[MESSAGE_SIZE];
    if (argc != ARGUMENTS_COUNT)
    {
        usage(argv[0]);
        return EXIT_FAILURE;
    }
    if (sethandler(SIG_IGN, SIGPIPE))
        ERR("Seting SIGPIPE:");
    if (sethandler(sigint_handler, SIGINT))
        ERR("Seting SIGINT:");
    fd = connect_tcp_socket(argv[1], argv[2]);
    doClient(fd);
    // prepare_request(argv,data);

    if (TEMP_FAILURE_RETRY(close(fd)) < 0)
        ERR("close");
    fprintf(stderr, "Client has terminated.\n");
    return EXIT_SUCCESS;
}