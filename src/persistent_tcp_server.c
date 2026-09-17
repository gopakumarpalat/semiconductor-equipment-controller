/*

Client
  │
  │ connect()
  ▼
Worker Thread
  │
  ├── recv START
  ├── send ACK
  │
  ├── recv STOP
  ├── send ACK
  │
  ├── recv RESET
  ├── send ACK
  │
  └── client disconnect

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>


#define PORT 5000

volatile sig_atomic_t server_running = 1; /* 1 → Server running, 0 → Server shutdown requested*/


void handle_sigint(int signal)
{
    (void)signal; // To avoid compiler warning
    printf("from handle_sigint\n");
    server_running = 0;
}


int recv_line( int client_fd, char *buffer, int buffer_size)
{
    int total = 0;

    while (total < buffer_size - 1)
    {
        char ch;

        int bytes_received = recv( client_fd, &ch, 1, 0);

        if (bytes_received == -1)
        {
            return -1;
        }

        if (bytes_received == 0)
        {
            return 0;
        }

        if (ch == '\n')
        {
            buffer[total] = '\0';
            return total;
        }

        buffer[total] = ch;
        total++;
    }

    buffer[total] = '\0';

    return total;
}


void *client_handler(void *arg)
{
    // Print current running thread ID
    printf( "Worker Thread ID: %lu\n", (unsigned long)pthread_self());

    //printf("Thread: sleeping for 10 seconds...\n");
    //sleep(10);


    int client_fd = *(int *)arg;
    
    // Release memory which cretaed using malloc, after copying to avoid memory leak
    free(arg);

    char buffer[1024];

    while (1)
    {

        int bytes_received;

        //bytes_received = recv( client_fd, buffer, sizeof(buffer), 0);
        bytes_received = recv_line( client_fd, buffer, sizeof(buffer));

        if (bytes_received == -1)
        {
            perror("recv_line");
            break;
        }
        else if (bytes_received == 0)
        {
             printf( "[Thread %lu] client disconnected.\n", (unsigned long)pthread_self());
            break;
        }

        printf( "[Thread %lu] received command: %s\n", (unsigned long)pthread_self(), buffer);

        char response[2048];

        snprintf( response, sizeof(response), "ACK %s\n", buffer);

        if (send( client_fd, response, strlen(response), 0) == -1)
        {
            perror("send");
            break;
        }

        printf( "[Thread %lu] response sent: %s", (unsigned long)pthread_self(), response);
    }

    close(client_fd);

    printf("Thread: connection closed.\n");

    return NULL;
}


int main(void)
{
    struct sigaction sa;

    memset( &sa, 0, sizeof(sa));

    sa.sa_handler = handle_sigint;

    sigaction( SIGINT, &sa, NULL);  // Handle sigint; SIGINT = Interrupt signal(Ctrl + C)

    int server_fd;

    struct sockaddr_in server_addr;

    server_fd = socket( AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1)
    {
        perror("socket");
        return 1;
    }

    printf("Server: socket created.\n");

    memset( &server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind( server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind");
        close(server_fd);
        return 1;
    }

    printf("Server: bind successful.\n");

    if (listen(server_fd, 5) == -1)
    {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf( "Server: waiting for clients on port %d...\n", PORT);

    printf( "Main Thread ID: %lu\n", (unsigned long)pthread_self());

    while (server_running)
    {
        int *client_fd = malloc(sizeof(int));

        if (client_fd == NULL)
        {
            perror("malloc");
            continue;
        }

        *client_fd = accept( server_fd, NULL, NULL);

        if (*client_fd == -1)
        {
            if (errno == EINTR)
            {
                printf("errno == EINTR\n");
                free(client_fd);
                break;
            }

            perror("accept");
            free(client_fd);
            continue;
        }

        printf("Server: client connected.\n");

        pthread_t thread;

        if (pthread_create( &thread, NULL, client_handler, client_fd) != 0)
        {
            perror("pthread_create");
            close(*client_fd);
            free(client_fd);
            continue;
        }
       
        /*
        * pthread_detach() → Thread runs independently, now wait for another thread to complete.
        * pthread_join() → Wait until thread finish.
        */
        pthread_detach(thread);

    }


    printf("Server: shutting down...\n");

    close(server_fd);

    printf("Server: server socket closed.\n");

    return 0;
}