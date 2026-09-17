/*
                    Server
                      │
                  server_fd
                      │
                   accept()
                      │
          ┌───────────┼───────────┐
          ▼           ▼           ▼
       Client 1    Client 2    Client 3
          │           │           │
       Thread 1    Thread 2    Thread 3
          │           │           │
       recv/send   recv/send   recv/send
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>

#define PORT 5000


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

    int bytes_received;

    bytes_received = recv( client_fd, buffer, sizeof(buffer), 0);

    if (bytes_received == -1)
    {
        perror("recv");
        close(client_fd);
        return NULL;
    }
    else if (bytes_received == 0)
    {
        printf("Thread: client disconnected.\n");
        close(client_fd);
        return NULL;
    }

    printf( "Thread: received command: %s\n", buffer);

    char response[] = "ACK START";

    if (send( client_fd, response, strlen(response) + 1, 0) == -1)
    {
        perror("send");
        close(client_fd);
        return NULL;
    }

    printf( "Thread: response sent: %s\n", response);

    close(client_fd);

    return NULL;
}


int main(void)
{
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

    while (1)
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


    close(server_fd);

    return 0;
}