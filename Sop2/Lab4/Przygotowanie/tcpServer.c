#include "soplib.h"

volatile sig_atomic_t do_work = 1;
struct Client *clients;
int clientsCount = 0;

void sigint_handler(int sig)
{
    do_work = 0;
}

struct Client
{
    int id;
    int L;
    int fd;
};

void usage(char *name)
{
    fprintf(stderr, "USAGE: %s socket port\n", name);
}

int tryAddClient(int cfd)
{

    if (clientsCount >= MAX_CLIENT)
    {
        printf("Too many clients accepted: %d\n", clientsCount);
        return -1;
    }
    for (int i = 0; i < MAX_CLIENT; i++)
    {
        if (-1 == clients[i].id)
        {
            // struct Client newClient;
            // newClient.fd = cfd;
            // clients[i] = *newClient;
            clients[i].id = i;
            clients[i].fd = cfd;
            clientsCount++;
            printf("Client added\n");
            return clients[i].id;
        }
    }
    ERR("Exited outside of the loop\n");
    return -1;
}

int tryDeleteClient(int clientId)
{
    for (int i = 0; i < MAX_CLIENT; i++)
    {
        if (clientId == clients[i].id)
        {
            clients[i].id = -1;
            clients[i].L = -1;
            return clientId;
        }
    }
    return -1;
}

void prepareFullErrorMessage(int32_t *data)
{
    data[0] = htonl('F');
    data[1] = 0;
    data[2] = 0;
}

int resolveMessageFromClient(int32_t data[MESSAGE_SIZE], int cfd)
{
    char message = ntohl(data[0]);
    struct Client *newClient = malloc(sizeof(struct Client));
    newClient->id = ntohl(data[1]);
    newClient->L = ntohl(data[2]);
    printf("Character received from client %c\n", message);
    // int result;
    switch (message)
    {
    case 'F':
        break;
    case 'N':
        // if (-1 == result)
        // {
        //     int32_t *data = malloc(sizeof(int32_t[MESSAGE_SIZE]));
        //     prepareFullErrorMessage(data);
        //     if (bulk_write(cfd, (char *)data, sizeof(int32_t[MESSAGE_SIZE])) < 0 && errno != EPIPE)
        //         ERR("write:");
        // }
        // return -1;
        break;
    case 'M':
        break;
    case 'H':
        break;
    default:
        printf("Unknown message received from server\n");
        return -1;
    }
    return 0;
}

int removeClient(int fd)
{
    for (int i = 0; i < MAX_CLIENT; i++)
    {
        if (clients[i].fd == fd)
        {
            clients[i].fd = -1;
            clients[i].id = -1;
            clients[i].L = NULL;
            return 0;
        }
    }
    return -1;
}

int sendResponse(int cfd)
{
    ssize_t size;

    int32_t data[MESSAGE_SIZE];
    if ((size = bulk_read(cfd, (char *)data, sizeof(int32_t[MESSAGE_SIZE]))) < 0)
        ERR("Reading message from sendResponse\n");
    if (size == (int)sizeof(int32_t[MESSAGE_SIZE]))
    {
        int result = resolveMessageFromClient(data, cfd);
        if (result == 0)
            if (bulk_write(cfd, (char *)data, sizeof(int32_t[MESSAGE_SIZE])) < 0 && errno != EPIPE)
                ERR("write:");
        if (result == -1)
            ERR("Did not send response message\n");
    }
    if (size == 0)
    {
        return -1;
    }
    return 0;
    // if ((size = bulk_read(cfd, (char *)data, sizeof(int32_t[MESSAGE_SIZE]))) < 0)
    //     ERR("Reading message from sendResponse\n");
    // if (TEMP_FAILURE_RETRY(close(cfd)) < 0)
    //     ERR("close");
}

void prepareClientsTable(int n)
{
    clients = malloc(n * sizeof(struct Client));
    for (int i = 0; i < n; i++)
    {
        clients[i].id = -1;
        clients[i].L = -1;
    }
}

void displayAllClients()
{
    for (int i = 0; i < MAX_CLIENT; i++)
    {
        printf("Client number %d: %d %d %d\n", i, clients[i].fd, clients[i].id, clients[i].L);
    }
}

void doServer(int fdT, fd_set *base_rfds)
{
    int cfd;
    fd_set rfds;
    sigset_t mask, oldmask;
    FD_ZERO(base_rfds);
    FD_SET(fdT, base_rfds);
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);
    while (do_work)
    {
        printf("I am in while\n");
        rfds = *base_rfds;
        // for(int i=0; i< MAX_CLIENT; i++)
        // {
        //     if(clients[i].id>=0)
        //     {
        //         printf("Client detected: %d\n", clients[i].id);
        //         sendResponse(clients[i].fd);
        //     }
        // }
        if (pselect(FD_SETSIZE, &rfds, NULL, NULL, NULL, &oldmask) > 0)
        {
            //I need to learn how to acutally use pselect() function
            if (FD_ISSET(fdT, &rfds))
            {

                cfd = add_new_client(fdT); //Here is problably the mistake

                if (cfd > 0)
                {
                    printf("Cfd >0\n");
                    FD_SET(cfd, base_rfds);
                    // struct Client newClient;
                    // newClient.fd = cfd;
                    int addedClientId = tryAddClient(cfd);
                    displayAllClients();
                    if (-1 == addedClientId)
                    {
                        int32_t *data = malloc(sizeof(int32_t[MESSAGE_SIZE]));
                        prepareFullErrorMessage(data);
                        if (bulk_write(cfd, (char *)data, sizeof(int32_t[MESSAGE_SIZE])) < 0 && errno != EPIPE)
                            ERR("write:");
                    }
                    printf("Id of an added client %d. Its fd %d\n", addedClientId, cfd);
                }
            }
            for (int i = 0; i < MAX_CLIENT; i++)
            {
                if (FD_ISSET(clients[i].fd, &rfds))
                {
                    int result = sendResponse(clients[i].fd);
                    if (result == -1)
                    {
                        FD_CLR(cfd, base_rfds);
                        if (TEMP_FAILURE_RETRY(close(clients[i].fd)) < 0)
                            ERR("close");
                        removeClient(clients[i].fd);
                    }
                }
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
    fd_set base_rfds;
    prepareClientsTable(MAX_CLIENT);
    int fdT;
    int new_flags;
    if (argc != 3)
    {
        usage(argv[0]);
        return EXIT_FAILURE;
    }
    if (sethandler(SIG_IGN, SIGPIPE))
        ERR("Seting SIGPIPE:");
    if (sethandler(sigint_handler, SIGINT))
        ERR("Seting SIGINT:");
    fdT = bind_tcp_socket(atoi(argv[2]), BACKLOG);
    new_flags = fcntl(fdT, F_GETFL) | O_NONBLOCK;
    fcntl(fdT, F_SETFL, new_flags);
    doServer(fdT, &base_rfds);
    if (TEMP_FAILURE_RETRY(close(fdT)) < 0)
        ERR("close");
    fprintf(stderr, "Server has terminated.\n");
    return EXIT_SUCCESS;
}