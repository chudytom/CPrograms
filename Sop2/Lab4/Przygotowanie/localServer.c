#include "soplib.h"

#define BACKLOG 3
volatile sig_atomic_t do_work = 1;

void sigint_handler(int sig)
{
    do_work = 0;
}
void usage(char *name)
{
    fprintf(stderr, "USAGE: %s socket port\n", name);
}

void calculate(int32_t data[5])
{
    int32_t op1, op2, result, status = 1;
    op1 = ntohl(data[0]);
    op2 = ntohl(data[1]);
    switch ((char)ntohl(data[3]))
    {
    case '+':
        result = op1 + op2;
        break;
    case '-':
        result = op1 - op2;
        break;
    case '*':
        result = op1 * op2;
        break;
    case '/':
        if (0 == op2)
            status = 0;
        else
            result = op1 / op2;
        break;
    default:
        status = 0;
    }
    data[4] = htonl(status);
    data[2] = htonl(result);
}

void doServer(int fdL)
{
    int cfd;
    int32_t data[5];
    ssize_t size;
    fd_set base_rfds, rfds;
    sigset_t mask, oldMask;
    FD_ZERO(&base_rfds);
    FD_SET(fdL, &base_rfds);
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigprocmask(SIG_BLOCK, &mask, &oldMask);
    while (do_work)
    {
        rfds = base_rfds;
        if (pselect(fdL + 1, &rfds, NULL, NULL, NULL, &oldMask) > 0)
        {
            if ((cfd = add_new_client(fdL)) >= 0)
            {
                if ((size = bulk_read(cfd, (char *)data, sizeof(int32_t[5]))) < 0)
                    ERR("read:");
                if (size == (int)sizeof(int32_t[5]))
                {
                    calculate(data);
                    if (bulk_write(cfd, (char *)data, sizeof(int32_t[5])) < 0 && EPIPE != errno)
                        ERR("write:");
                }
                if (TEMP_FAILURE_RETRY(close(cfd)) < 0)
                    ERR("close: ");
            }
        }
        else
        {
            if (EINTR == errno)
                continue;
            ERR("pselect");
        }
    }
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

int main(int argc, char **argv)
{
    int fdL;
    int new_flags;
    if (argc != 3)
    {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (sethandler(SIG_IGN, SIGPIPE))
        ERR("Setting SIGPIPE:");
    if (sethandler(sigint_handler, SIGINT))
        ERR("Setting SIGINT:");
    fdL = bind_socket(argv[1], BACKLOG);
    new_flags = fcntl(fdL, F_GETFL) | O_NONBLOCK;
    fcntl(fdL, F_SETFL, new_flags);
    doServer(fdL);
    if (TEMP_FAILURE_RETRY(close(fdL)) < 0)
        ERR("close:");
    if (unlink(argv[1]) < 0)
        ERR("close");
    fprintf(stderr, "Server has terminated.\n");
    return EXIT_SUCCESS;
}