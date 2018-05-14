#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#define ERR(source) (fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), \
                     perror(source), kill(0, SIGKILL),               \
                     exit(EXIT_FAILURE))

#define MAX_BUFF 200
#define MessageSize PIPE_BUF
#define MIN_NUMBER 0
#define MAX_NUMBER 99
volatile sig_atomic_t lastSignal = 0;

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

void sigintParent(int sig)
{
    printf("SIGINT received in parent\n");
    lastSignal = sig;
}

void sigintChild(int sig)
{
    printf("SIGINT received in child\n");
    if (0 == (rand() % 5))
        exit(EXIT_SUCCESS);
}

void customSleep(int s, int nanoS)
{
    struct timespec t1, tRemaining;
    t1.tv_sec = s;
    t1.tv_nsec = nanoS;

    if (nanosleep(&t1, &tRemaining) < 0)
        ERR("Nanosleep");
}

void sendNumber(int *fds)
{

    customSleep(0, 20000000);

    char c = MIN_NUMBER + rand() % (MAX_NUMBER + 1);

    char buffer[MessageSize];
    char *bufHelper;
    *((pid_t *)buffer) = getpid();

    bufHelper = buffer + sizeof(pid_t);

    // printf("Trying to send number %c", c);
    int count = -1;
    // bufHelper[0] = 18;
    memset(bufHelper, c, 1);

    memset(bufHelper + 1, -1, MessageSize - sizeof(pid_t) - 1);
    if (write(fds[1], buffer, MessageSize) <= 0)
        ERR("Write to pipe in child \n");

    if ((count = read(fds[0], buffer, MessageSize)) < 0)
        ERR("Reading form file");
    if (count > 0)
    {
        printf("\nMy PID: %d\n", getpid());
        printf(" Message from PID: %d \n", *((pid_t *)buffer));
        for (int i = sizeof(pid_t); i < MessageSize; i++)
        {
            if (-1!= buffer[i])
                printf("%d", buffer[i]);
        }
        printf("\n");
    }
}

void childWork(int *fds, int childNumber)
{

    srand(getpid());
    int n = 2;
    int nMax = 2;

    sendNumber(fds);
    // if (count < MessageSize)

    while (n--)
    {
        if (fds[n] && close(fds[n]) < 0)
            ERR("Close fds in main");
    }

    printf("Child number %d closed %d descriptors\n", childNumber, nMax);

    printf("Child number %d exiting\n", childNumber);
    exit(EXIT_SUCCESS);
}

void parentWork(int *fds)
{
    srand(getpid());

    int status;
    char buff[PIPE_BUF];

    sendNumber(fds);

    for (;;)
    {

        status = read(fds[0], buff, 1);
        if (status < 0 && EINTR == errno)
            continue;
        if (status < 0)
            ERR("Read");
        if (0 == status)
        {
            printf("Break in the parent loop\n");
            break;
        }
    }
}

void createChildrenAndPipes(int *fdsParent)
{
    int pipe1[2];
    int pipe2[2];
    int pipe3[2];

    int fdsChild1[2];
    int fdsChild2[2];

    if (pipe(pipe1))
    {
        ERR("Creating pipe1");
    }
    if (pipe(pipe2))
    {
        ERR("Creating pipe2");
    }
    if (pipe(pipe3))
    {
        ERR("Creating pipe3");
    }

    fdsParent[0] = pipe1[0];
    printf("Parent gets read from pipe1\n");
    fdsParent[1] = pipe2[1];
    printf("Parent gets write from pipe2\n");

    // Fork for child1
    switch (fork())
    {
    case 0:
        fdsChild1[0] = pipe3[0];
        printf("Child1 gets read from pipe3\n");
        fdsChild1[1] = pipe1[1];
        printf("Child1 gets write from pipe1\n");

        if (close(pipe3[1]) < 0 || close(pipe1[0]) < 0 || close(pipe2[0]) < 0 || close(pipe2[1]) < 0)
            ERR("Close unused parent fds");
        printf("Child1 closes write from pipe3\n");
        printf("Child1 closes read from pipe1\n");
        printf("Child1 closes read from pipe2\n");
        printf("Child1 closes write from pipe2\n");

        childWork(fdsChild1, 1);

        printf("Child1 finishing\n");
        exit(EXIT_SUCCESS);
    case -1:
        ERR("Fork");
    }

    customSleep(0, 20000000);

    // Fork for child2
    switch (fork())
    {
    case 0:
        fdsChild2[0] = pipe2[0];
        printf("Child2 gets read from pipe2\n");
        fdsChild2[1] = pipe3[1];
        printf("Child2 gets write from pipe3\n");

        if (close(pipe2[1]) < 0 || close(pipe3[0]) < 0 || close(pipe1[0]) < 0 || close(pipe1[1]) < 0)
            ERR("Close unused parent fds");
        printf("Child2 closes write from pipe2\n");
        printf("Child2 closes read from pipe3\n");
        printf("Child2 closes read from pipe1\n");
        printf("Child2 closes write from pipe1\n");

        childWork(fdsChild2, 2);

        printf("Child2 finishing\n");
        exit(EXIT_SUCCESS);
    case -1:
        ERR("Fork");
    }

    if (close(pipe1[1]) < 0 || close(pipe2[0]) < 0 || close(pipe3[0]) < 0 || close(pipe3[1]) < 0)
        ERR("Close unused parent fds");
    printf("Parent closes write from pipe1\n");
    printf("Parent closes read from pipe2\n");
    printf("Parent closes read from pipe3\n");
    printf("Parent closes write from pipe3\n");
}

void usage(char *programName)
{
    fprintf(stderr, "USAGE: %s \n", programName);
    fprintf(stderr, "No arguments\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    if (argc != 1)
        usage(argv[0]);

    int n = 2;

    // if (sethandler(sigchld_handler, SIGCHLD) < 0)
    //     ERR("Sigchild");
    // if (sethandler(SIG_IGN, SIGINT) < 0)
    //     ERR("Setting SIGINT error");
    if (sethandler(SIG_IGN, SIGPIPE))
        ERR("Setting SIGPIPE error");
    int *fdsParent;
    if (NULL == (fdsParent = (int *)malloc(sizeof(int) * n)))
        ERR("Malloc");
    // if (pipe(R) < 0)
    //     ERR("Creating R pipe");
    createChildrenAndPipes(fdsParent);

    customSleep(0, 20000000);
    parentWork(fdsParent);
    // if (close(R[1]) < 0)
    //     ERR("Close R1");
    // parentWork(fdsParent);

    while (n--)
    {
        if (fdsParent[n] && close(fdsParent[n]) < 0)
            ERR("Close fds in main");
    }

    printf("Parent closed 2 descriptors\n");

    free(fdsParent);
    printf("Parent exiting\n");
    return EXIT_SUCCESS;
}
