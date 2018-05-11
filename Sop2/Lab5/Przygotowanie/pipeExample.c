#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#define ERR(source) (fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), \
                     perror(source), kill(0, SIGKILL),               \
                     exit(EXIT_FAILURE))

int sethandler(void (*f)(int), int sigNo)
{
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = f;
    if (-1 == sigaction(sigNo, &act, NULL))
        return -1;
    return 0;
}

void sigchld_handler(int sig)
{
    pid_t pid;
    for (;;)
    {
        pid = waitpid(0, NULL, WNOHANG);
        if (0 == pid)
            return;
        if (0 >= pid)
        {
            if (ECHILD == errno)
                return;
            ERR("waitpid:");
        }
    }
}

childWork(int R, int fd)
{
    srand(getpid());

    if (write(R, ('a' + ('z' - 'a') * rand()), 1) < 0)
        ERR("Write");
    exit(EXIT_SUCCESS);
}

parentWork(int R, int *fds)
{
    char buf[PIPE_BUF];
    int status;
    srand(getpid());
    while (1 == (status = read(R, buf, 1)))
    {
        printf("%c", buf[0]);
    }
}

createChildrenAndPipes(int R, int *fds, int n)
{
    int tempFds[2];
    while (n)
    {
        pipe(tempFds);
        switch (fork())
        {
        case 0:
            childWork(R, tempFds[0]);
            free(fds);
        case -1:
            ERR("Fork");
        }

        if (close(tempFds[0]) < 0)
            ERR("Close tempFds");
        fds[n] = tempFds[1];
        n--;
    }
}

void usage(char *programName)
{
    fprintf(stderr, "USAGE: %s n\n", programName);
    fprintf(stderr, "1<=n<=10 : n - number of clients\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    int R[2];

    if (argc != 2)
        usage(argv[0]);

    int n = atoi(argv[1]);
    if (n < 1 || n > 10)
        usage(argv[0]);

    if (sethandler(sigchld_handler, SIGCHLD) < 0)
        ERR("Sigchild");

    int *fds = malloc(sizeof(int) * n);
    pipe(R);
    createChildrenAndPipes(R[1], fds, n);
    parentWork(R[0], fds);

    if (close(R[0]) < 0)
        ERR("Close R0");
    if (close(R[1]) < 0)
        ERR("Close R1");
}
