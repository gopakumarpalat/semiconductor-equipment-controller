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
#include "equipment.h"

#define PORT 5000

typedef struct
{
    int client_fd;
    Equipment *equipment;

} ClientContext;

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
    ClientContext *context = (ClientContext *)arg;

    int client_fd = context->client_fd;

    Equipment *equipment = context->equipment;

    free(context);


    printf(
        "[Thread %lu] Worker thread started.\n",
        (unsigned long)pthread_self()
    );


    char buffer[1024];


    while (1)
    {
        int bytes_received;

        bytes_received = recv_line(
            client_fd,
            buffer,
            sizeof(buffer)
        );


        if (bytes_received == -1)
        {
            perror("recv_line");
            break;
        }


        if (bytes_received == 0)
        {
            printf(
                "[Thread %lu] Client disconnected.\n",
                (unsigned long)pthread_self()
            );

            break;
        }


        printf(
            "[Thread %lu] Received command: %s\n",
            (unsigned long)pthread_self(),
            buffer
        );


        char response[2048];


        /*
         * START
         */

        if (strcmp(buffer, "START") == 0)
        {
            equipment_start(equipment);

            snprintf(
                response,
                sizeof(response),
                "ACK START\n"
            );
        }


        /*
         * STOP
         */

        else if (strcmp(buffer, "STOP") == 0)
        {
            equipment_stop(equipment);

            snprintf(
                response,
                sizeof(response),
                "ACK STOP\n"
            );
        }


        /*
         * STATUS
         */

        else if (strcmp(buffer, "STATUS") == 0)
        {
            pthread_mutex_lock(&equipment->mutex);

            EquipmentState state = equipment->state;

            pthread_mutex_unlock(&equipment->mutex);


            if (state == EQUIPMENT_IDLE)
            {
                snprintf(
                    response,
                    sizeof(response),
                    "STATE IDLE\n"
                );
            }
            else if (state == EQUIPMENT_READY)
            {
                snprintf(
                    response,
                    sizeof(response),
                    "STATE READY\n"
                );
            }
            else if (state == EQUIPMENT_RUNNING)
            {
                snprintf(
                    response,
                    sizeof(response),
                    "STATE RUNNING\n"
                );
            }
            else
            {
                snprintf(
                    response,
                    sizeof(response),
                    "STATE UNKNOWN\n"
                );
            }
        }


        /*
         * RESET
         */

        else if (strcmp(buffer, "RESET") == 0)
        {
            pthread_mutex_lock(&equipment->mutex);

            equipment->state = EQUIPMENT_IDLE;

            pthread_mutex_unlock(&equipment->mutex);


            snprintf(
                response,
                sizeof(response),
                "ACK RESET\n"
            );
        }


        /*
         * Unknown command
         */

        else
        {
            snprintf(
                response,
                sizeof(response),
                "NACK UNKNOWN COMMAND\n"
            );
        }


        /*
         * Send response
         */

        if (send(
                client_fd,
                response,
                strlen(response),
                0) == -1)
        {
            perror("send");
            break;
        }


        printf(
            "[Thread %lu] Response sent: %s",
            (unsigned long)pthread_self(),
            response
        );
    }


    close(client_fd);


    printf("[Thread %lu] Connection closed.\n", (unsigned long)pthread_self()
    );


    return NULL;
}




int main(void)
{

    Equipment equipment;

    equipment_create( &equipment, 101, "ETCH01" );

    equipment_init(&equipment);
    
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
        int accepted_fd;

        accepted_fd = accept( server_fd, NULL, NULL );

        if (accepted_fd == -1)
        {
            if (errno == EINTR)
            {
                printf("errno == EINTR\n");
                break;
            }

            perror("accept");

            continue;
        }

        printf("Server: client connected.\n");

        ClientContext *context =  malloc(sizeof(ClientContext));

        if (context == NULL)
        {
            perror("malloc");
            close(accepted_fd);
            continue;
        }

        context->client_fd = accepted_fd;

        context->equipment = &equipment;

        pthread_t thread;

        if (pthread_create( &thread, NULL, client_handler, context) != 0)
        {
            perror("pthread_create");
            close(accepted_fd);
            free(context);
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

    equipment_destroy(&equipment);

    return 0;
}