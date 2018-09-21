#define _GNU_SOURCE
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

#define MAX_BUFF 200
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

void childWork(int R, int fd)
{
    srand(getpid());
    char message[MAX_BUFF + 1];
    char c;
    if (sethandler(sigintChild, SIGINT) < 0)
        ERR("Error setting SIGINT in child");
    for (;;)
    {
        printf("Child loop\n");
        if ((TEMP_FAILURE_RETRY(read(fd, &c, 1)) < 1))
            ERR("Read");
        printf("Characted %c received in process number %d\n", c, getpid());
        int times = 1 + rand() % MAX_BUFF;
        message[0] = times;
        memset(message + 1, c, times);
        if (TEMP_FAILURE_RETRY(write(R, message, times + 1)) < 0)
            ERR("Write");
    }
    exit(EXIT_SUCCESS);
}

void parentWork(int R, int *fds, int n)
{
    char buf[MAX_BUFF];
    int status;
    srand(getpid());
    printf("\n"); /*  */
    char c;
    if (sethandler(sigintParent, SIGINT) < 0)
        ERR("Error setting SIGINT in parent");

    for (;;)
    {
        if (SIGINT == lastSignal)
        {
            printf("SIGINT if in parent\n");
            int selectedPipeIndex = rand() % n;
            while (0 == fds[selectedPipeIndex % n] && selectedPipeIndex < 2 * n)
                selectedPipeIndex++;
            selectedPipeIndex %= n;
            if (fds[selectedPipeIndex])
            {
                c = 'a' + rand() % ('z' - 'a');

                status = TEMP_FAILURE_RETRY(write(fds[selectedPipeIndex], &c, 1));
                if (status != 1)
                {
                    printf("Closing fd number %d\n", selectedPipeIndex);
                    if (TEMP_FAILURE_RETRY(close(fds[selectedPipeIndex])) < 0)
                        ERR("Close ");
                    fds[selectedPipeIndex] = 0;
                }
            }
            lastSignal = 0;
        }

        status = read(R, &c, 1);
        if (status < 0 && EINTR == errno)   
            continue;
        if (status < 0)
            ERR("Read");
        if (0 == status)
        {
            printf("Break in the parent loop\n");
            break;
        }
        printf("Should have received %d bytes\n", c);
        if (TEMP_FAILURE_RETRY(read(R, buf, c)) < c)
            ERR("Read");
        buf[(int)c] = 0;
        printf("\n%s\n", buf);
    }
}

void createChildrenAndPipes(int R, int *fds, int n)
{
    int tempFds[2];
    int nMax = n;
    while (n)
    {
        if (pipe(tempFds))
        {
            char pipeErrorMessage[50] = "Creating pipe for process: ";
            const int bufferLength = 2;
            char number[bufferLength];
            snprintf(number, bufferLength, "%d", n);
            strcat(pipeErrorMessage, number);
            ERR(pipeErrorMessage);
        }
        switch (fork())
        {
        case 0:
            while (n < nMax)
            {
                if (fds[n] && close(fds[n]))
                    ERR("Close fds");
                n++;
            }
            free(fds);
            if (close(tempFds[1]))
                ERR("Close tempFds");
            childWork(R, tempFds[0]);
            if (close(tempFds[0]))
                ERR("Close tempFds");
            if (close(R) < 0)
                ERR("Close R");
            EXIT_SUCCESS;
        case -1:
            ERR("Fork");
        }

        if (close(tempFds[0]) < 0)
            ERR("Close tempFds");
        n--;
        fds[n] = tempFds[1];
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
    if (sethandler(SIG_IGN, SIGINT) < 0)
        ERR("Setting SIGINT error");
    if (sethandler(SIG_IGN, SIGPIPE))
        ERR("Setting SIGPIPE error");
    int *fds;
    if (NULL == (fds = (int *)malloc(sizeof(int) * n)))
        ERR("Malloc");
    if (pipe(R) < 0)
        ERR("Creating R pipe");
    createChildrenAndPipes(R[1], fds, n);
    if (close(R[1]) < 0)
        ERR("Close R1");
    parentWork(R[0], fds, n);
    while (n--)
    {
        if (fds[n] && close(fds[n]) < 0)
            ERR("Close fds in main");
    }

    if (close(R[0]) < 0)
        ERR("Close R0");

    free(fds);
    return EXIT_SUCCESS;
}
