// #include <unistd.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <errno.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>
// #include <ctype.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <limits.h>

#define ERR(source) (perror(source),                                 \
                     fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), \
                     exit(EXIT_FAILURE))

void usage(char *name)
{
    fprintf(stderr, "USAGE: %s fifoFile\n", name);
    exit(EXIT_FAILURE);
}

void readFromFifo(int fifo)
{
    int count;
    char buff[PIPE_BUF];    // This is so freaking important Whenever you can use static memory allocation
    do
    {
        count = read(fifo, buff, PIPE_BUF);
        if (count > 0)
        {
            printf("\nPID: %d \n", *((pid_t *)buff));
            printf("\n");
            for (int i = sizeof(pid_t); i < PIPE_BUF; i++)
            {
                if (isalnum(buff[i]))
                    printf("%c", buff[i]);
            }
        }
    } while (count > 0);
}

int cleanFifo(int fifo, char *fifoName)
{
    if (close(fifo) < 0)
    {
        ERR("Close fifo error");
        return -1;
    }
    if (unlink(fifoName) < 0)
        ERR("Remove fifo");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        usage(argv[0]);

    int fifo;
    if (mkfifo(argv[1], S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP) < 0)
        if (errno != EEXIST)
            ERR("create fifo");
    if ((fifo = open(argv[1], O_RDONLY)) < 0)
    {
        cleanFifo(fifo, argv[1]);
        ERR("Open fifo");
    }

    readFromFifo(fifo);

    cleanFifo(fifo, argv[1]);

    return EXIT_SUCCESS;
}
