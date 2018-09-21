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
            if (-1 != buffer[i])
                printf("%d", buffer[i]);
        }
        printf("\n");
    }
}

void childWork(int rfd, int wfd)
{
    customSleep(0, 20000000);
    // srand(getpid());
    // int n = 2;
    // int nMax = 2;

    // sendNumber(fds);
    // // if (count < MessageSize)

    // while (n--)
    // {
    //     if (fds[n] && close(fds[n]) < 0)
    //         ERR("Close fds in main");
    // }

    // printf("Child number %d closed %d descriptors\n", childNumber, nMax);

    // printf("Child number %d exiting\n", childNumber);
    exit(EXIT_SUCCESS);
}

sendMessage(int n, int *wds, int *processes)
{
    int p = rand() % n;
    int p2 = rand() % n;
    int num = rand();
    char buff[PIPE_BUF];

    buff[0] = p;
    buff[1] = p2;
    if (write(wds[p], buff, 2) < 0)
        ERR("Write");
    
}

void parentWork(int *wds, int *processes, int n)
{
    srand(getpid());
    if (sethandler(sigintParent, SIGINT) < 0)
        ERR("Error setting SIGINT in parent");

    for (;;)
    {
        if (SIGINT == lastSignal)
        {
            printf("SIGINT if in parent\n");

            lastSignal = 0;
        }
    }
}

void createChildrenAndPipes(int n, int *rds, int *wds, int *processes)
{
    int status;
    int tempFds[2];

    pid_t pid;
    wait(&status);

    int lastFds[2];
    pipe(lastFds);
    int firstWrite = lastFds[1];
    for (int i = 0; i < n; i++)
    {
        wds[i] = lastFds[1];
        if (n - i > 1)
        {
            pipe(tempFds);
        }
        else
        {
            tempFds[1] = firstWrite;
        }
        switch (pid = fork())
        {
        case 0:
            processes[i] = pid;
            int rMine = lastFds[0];
            int wMine = tempFds[1];
            int k = i;
            while (k >= 0)
            {
                if (wds[(k + 1) % n] && close(wds[(k + 1) % n]) < 0)
                    ERR("Close in loop");
                k--;
            }
            if (close(tempFds[0]))
                ERR("Close write fds");
            printf("Child number %d is running\n", i);
            childWork(rMine, wMine);
            exit(EXIT_SUCCESS);
        case -1:
            ERR("Fork");
        }
        if (close(lastFds[0]) < 0)
            ERR("Close outside of loop");
        lastFds[1] = tempFds[1];
        lastFds[0] = tempFds[0];
        waitpid(pid, &status, 0);
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
    if (argc != 2)
        usage(argv[0]);

    int n = atoi(argv[1]);
    if (n < 2 || n > 10)
        usage(argv[0]);

    int *rds;
    int *wds;
    int *processes;

    // if (sethandler(sigchld_handler, SIGCHLD) < 0)
    //     ERR("Sigchild");
    // if (sethandler(SIG_IGN, SIGINT) < 0)
    //     ERR("Setting SIGINT error");
    if (sethandler(sigintParent, SIGINT) < 0)
        ERR("Error setting SIGINT in parent");
    if (sethandler(SIG_IGN, SIGPIPE))
        ERR("Setting SIGPIPE error");
    if (NULL == (rds = (int *)malloc(sizeof(int) * n)))
        ERR("Malloc");

    if (NULL == (wds = (int *)malloc(sizeof(int) * n)))
        ERR("Malloc");

    if (NULL == (processes = (int *)malloc(sizeof(int) * n)))
        ERR("Malloc");
    // if (pipe(R) < 0)
    //     ERR("Creating R pipe");
    createChildrenAndPipes(n, rds, wds, processes);

    // customSleep(0, 20000000);
    parentWork(wds, processes, n);
    // if (close(R[1]) < 0)
    //     ERR("Close R1");
    // parentWork(fdsParent);

    printf("Parent closed 2 descriptors\n");

    int k = n;
    while (k-- > 0)
    {
        if (close(wds[k]) < 0)
            ERR("Close in main");
    }

    free(rds);
    free(wds);
    free(processes);
    printf("Parent exiting\n");
    return EXIT_SUCCESS;
}
