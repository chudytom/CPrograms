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
    char *buff = malloc(20 * sizeof(char));

    do
    {
        count = read(fifo, buff, 1);
        if (isalnum(buff[0]))
            printf("%c", buff[0]);
    } while (count > 0);
}

int cleanFifo(int fifo)
{
    if (close(fifo) < 0)
    {
        // ERR("Close fifo error");
        return -1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        usage(argv[0]);

    int fifo;
    if (mkfifo(argv[1], S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP) < 0)
        if(errno!=EEXIST) ERR("create fifo");
    if ((fifo = open(argv[1], O_RDONLY)) < 0)
    {
        cleanFifo(fifo);
        ERR("Open fifo");
    }

    readFromFifo(fifo);

    cleanFifo(fifo);

    return EXIT_SUCCESS;
}
