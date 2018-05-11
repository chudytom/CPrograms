#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>

#define MessageSize (PIPE_BUF - sizeof(pid_t))

#define ERR(source) (perror(source),                                 \
                     fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), \
                     exit(EXIT_FAILURE))

void usage(char *programName)
{
    fprintf(stderr, "USAGE: %s fifoFile fileToRead\n", programName);
    exit(EXIT_FAILURE);
}

void writeToFifo(int fifo, int file)
{
    int count;
    char buffer[PIPE_BUF];
    char *bufHelper;
    *((pid_t *)buffer) = getpid();
    bufHelper = buffer + sizeof(pid_t);
    do
    {
        if ((count = read(file, bufHelper, MessageSize)) < 0)
            ERR("Reading form file");
        if (count < MessageSize)
            memset(bufHelper + count,0, MessageSize - count);
        if (count > 0)
        {
            write(fifo, buffer, PIPE_BUF);
        }
    } while (MessageSize == count);
}

int main(int argc, char **argv)
{
    if (argc != 3)
        usage(argv[0]);

    if (mkfifo(argv[1], S_IRGRP | S_IWGRP | S_IWUSR | S_IRUSR) < 0)
        if (errno != EEXIST)
            ERR("Make fifo");

    int file, fifo;
    if ((fifo = open(argv[1], O_WRONLY)) < 0)
        ERR("Open fifo");
    if ((file = open(argv[2], O_RDONLY)) < 0)
        ERR("Open file");

    writeToFifo(fifo, file);

    if (close(file) < 0)
        ERR("Close file");

    if (close(fifo) < 0)
        ERR("Close fifo");

    return EXIT_SUCCESS;
}