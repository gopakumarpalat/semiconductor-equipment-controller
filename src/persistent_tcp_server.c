/**
 * @file persistent_tcp_server.c
 *
 * @brief Persistent multi-threaded TCP server for semiconductor equipment control.
 *
 * @details
 * This module implements a TCP server that accepts client connections and
 * processes equipment control commands using a dedicated worker thread
 * for each connected client.
 *
 * Supported commands:
 *   - START
 *   - STOP
 *   - STATUS
 *   - RESET
 *   - ALARM
 *
 * The server communicates with clients using a simple line-based TCP protocol.
 * Each command is terminated by a newline character ('\n').
 *
 * The server integrates with the Equipment module for:
 *   - Equipment state management
 *   - Start/Stop callbacks
 *   - Alarm callbacks
 *   - Thread-safe state access using a mutex
 *
 * Architecture:
 *
 *   TCP Client
 *       |
 *       | connect()
 *       v
 *   TCP Server (Main Thread)
 *       |
 *       | accept()
 *       v
 *   Worker Thread
 *       |
 *       +-- recv START  --> equipment_start()
 *       |                    |
 *       |                    +--> Start Callback
 *       |
 *       +-- recv STOP   --> equipment_stop()
 *       |                    |
 *       |                    +--> Stop Callback
 *       |
 *       +-- recv STATUS --> Read equipment state
 *       |
 *       +-- recv RESET  --> Reset equipment state
 *       |
 *       +-- recv ALARM  --> equipment_raise_alarm()
 *                            |
 *                            +--> Alarm Callback
 *
 * @note
 * This implementation uses POSIX sockets and pthreads and is intended
 * to run on Linux/WSL.
 *
 * @author Gopakumar Palat
 * @date 2026-09-18
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

/*===========================================================================
 * Constants
 *===========================================================================*/

/**
 * @brief TCP port used by the equipment control server.
 */
#define PORT 5000

/*===========================================================================
 * Data Types
 *===========================================================================*/

/**
 * @brief Context information passed to a client worker thread.
 *
 * Each connected client gets a dedicated worker thread. This structure
 * contains the information required by that worker thread to communicate
 * with the client and access the equipment.
 */
typedef struct
{
    int client_fd;          /**< Connected client socket descriptor. */
    Equipment *equipment;   /**< Pointer to the equipment instance. */

} ClientContext;


/*===========================================================================
 * Global Variables
 *===========================================================================*/

/**
 * @brief Controls the main server loop.
 *
 * Value is 1 while the server is running.
 * Value is set to 0 when shutdown is requested using SIGINT (Ctrl+C).
 */
volatile sig_atomic_t server_running = 1;


/*===========================================================================
 * Signal Handler
 *===========================================================================*/

/**
 * @brief Handles the SIGINT signal.
 *
 * This function is called when the user presses Ctrl+C.
 * It requests a graceful server shutdown by clearing server_running.
 *
 * @param signal Signal number received.
 *
 * @return None.
 */
void handle_sigint(int signal)
{
    (void)signal;   /* Avoid unused parameter warning. */

    printf("from handle_sigint\n");

    server_running = 0;
}



/*===========================================================================
 * Equipment Callback Functions
 *===========================================================================*/

/**
 * @brief Handle generic equipment events.
 *
 * @param equipment Equipment that generated the event.
 * @param event     Type of equipment event.
 * @param message   Optional event message.
 */
void on_equipment_event( Equipment *equipment, EquipmentEvent event, const char *message) 
{
     printf( ">>> EVENT: %s %s: %s\n", equipment->name, equipment_event_name(event), message );
}


/*===========================================================================
 * TCP Receive Functions
 *===========================================================================*/

/**
 * @brief Receives one line of text from a TCP client.
 *
 * The function receives data one byte at a time until a newline character
 * is received. The newline character is not stored in the output buffer.
 * The received string is always null-terminated when a complete line
 * is received.
 *
 * @param client_fd   Connected client socket descriptor.
 * @param buffer      Destination buffer for the received line.
 * @param buffer_size Size of the destination buffer.
 *
 * @return
 *   >= 0 : Number of characters received before '\n'.
 *   -1   : Receive error.
 *    0   : Client disconnected.
 */
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

    /*
     * Buffer is full.
     * Null-terminate the string to keep it safe for string operations.
     */
    buffer[total] = '\0';

    return total;
}




/*===========================================================================
 * Client Worker Thread
 *===========================================================================*/

/**
 * @brief Handles communication with one connected TCP client.
 *
 * A separate worker thread is created for each connected client.
 * The worker continuously receives commands, executes the corresponding
 * equipment operation, and sends an acknowledgement or response.
 *
 * Supported commands:
 *   - START
 *   - STOP
 *   - STATUS
 *   - RESET
 *   - ALARM
 *
 * @param arg Pointer to a ClientContext structure.
 *
 * @return NULL when the client connection is closed.
 */
void *client_handler(void *arg)
{
    ClientContext *context = (ClientContext *)arg;

    int client_fd = context->client_fd;

    Equipment *equipment = context->equipment;

    /*
     * ClientContext was dynamically allocated by the main thread.
     * The worker thread owns it now, so release it after extracting
     * the required information.
     */
    free(context);

    printf( "[Thread %lu] Worker thread started.\n", (unsigned long)pthread_self() );

    char buffer[1024];

    /*
     * Keep the TCP connection open and process multiple commands
     * from the same client.
     */
    while (1)
    {
        int bytes_received;

        /*
         * Receive one complete command.
         */
        bytes_received = recv_line( client_fd, buffer, sizeof(buffer));

        /*
        * Receive error.
        */
        if (bytes_received == -1)
        {
            perror("recv_line");
            break;
        }

        /*
         * Client closed the connection.
         */
        if (bytes_received == 0)
        {
            printf( "[Thread %lu] Client disconnected.\n", (unsigned long)pthread_self());
            break;
        }

        printf( "[Thread %lu] Received command: %s\n", (unsigned long)pthread_self(), buffer );

        char response[2048];

        /*-------------------------------------------------------------------
         * START command
         *------------------------------------------------------------------*/
        if (strcmp(buffer, "START") == 0)
        {
            equipment_start(equipment);

            snprintf( response, sizeof(response), "ACK START\n" );
        }

        /*-------------------------------------------------------------------
         * STOP command
         *------------------------------------------------------------------*/
        else if (strcmp(buffer, "STOP") == 0)
        {
            equipment_stop(equipment);

            snprintf( response, sizeof(response), "ACK STOP\n");
        }

        /*-------------------------------------------------------------------
         * STATUS command
         *------------------------------------------------------------------*/
        else if (strcmp(buffer, "STATUS") == 0)
        {
            /*
             * Equipment state is shared between worker threads.
             * Protect the read operation using the equipment mutex.
             */
            pthread_mutex_lock(&equipment->mutex);

            EquipmentState state = equipment->state;

            pthread_mutex_unlock(&equipment->mutex);

            if (state == EQUIPMENT_IDLE)
            {
                snprintf( response, sizeof(response), "STATE IDLE\n" );
            }
            else if (state == EQUIPMENT_READY)
            {
                snprintf( response, sizeof(response), "STATE READY\n" );
            }
            else if (state == EQUIPMENT_RUNNING)
            {
                snprintf( response, sizeof(response), "STATE RUNNING\n" );
            }
            else
            {
                snprintf( response, sizeof(response), "STATE UNKNOWN\n" );
            }
        }

        /*-------------------------------------------------------------------
         * RESET command
         *------------------------------------------------------------------*/
        else if (strcmp(buffer, "RESET") == 0)
        {
            /*
             * Equipment state is shared between worker threads,
             * therefore the state update must be protected by the mutex.
             */
            pthread_mutex_lock(&equipment->mutex);

            equipment->state = EQUIPMENT_IDLE;

            pthread_mutex_unlock(&equipment->mutex);

            snprintf( response, sizeof(response), "ACK RESET\n" );
        }

        /*-------------------------------------------------------------------
         * ALARM command
         *------------------------------------------------------------------*/
        else if (strcmp(buffer, "ALARM") == 0)
        {
            /*
             * Raise an equipment alarm.
             *
             * equipment_raise_alarm() will invoke the registered alarm callback.
             */
            equipment_raise_alarm( equipment, "Temperature too high" );

            snprintf( response, sizeof(response), "ACK ALARM\n" );
        }

        /*-------------------------------------------------------------------
         * Unknown command
         *------------------------------------------------------------------*/
        else
        {
            snprintf( response, sizeof(response), "NACK UNKNOWN COMMAND\n" );
        }

        /*-------------------------------------------------------------------
         * Send response to client
         *------------------------------------------------------------------*/
        if (send( client_fd, response, strlen(response), 0) == -1)
        {
            perror("send");
            break;
        }

        printf( "[Thread %lu] Response sent: %s", (unsigned long)pthread_self(), response);
    }

    /*
     * Close the connected client socket when the worker thread finishes.
     */
    close(client_fd);

    printf("[Thread %lu] Connection closed.\n", (unsigned long)pthread_self());

    return NULL;
}



/*===========================================================================
 * Main Function
 *===========================================================================*/

/**
 * @brief Entry point of the persistent TCP server.
 *
 * Initializes the equipment, registers equipment callbacks, creates
 * the TCP listening socket, accepts client connections, and creates
 * a worker thread for each connected client.
 *
 * The server continues running until SIGINT (Ctrl+C) is received.
 *
 * @return
 *   0 : Server terminated successfully.
 *   1 : Server initialization error.
 */
int main(void)
{

    /*-----------------------------------------------------------------------
     * Equipment initialization
     *-----------------------------------------------------------------------*/
    Equipment equipment;

    equipment_create( &equipment, 101, "ETCH01" );

    /*
     * Register equipment callbacks.
     *
     * These callbacks are invoked by the Equipment module when the
     * corresponding events occur.
     */
    equipment_set_event_callback( &equipment, on_equipment_event);

    /*
     * Move equipment from IDLE to READY.
     */
    equipment_init(&equipment);

    /*-----------------------------------------------------------------------
     * Signal handling
     *-----------------------------------------------------------------------*/
    
    struct sigaction sa;

    memset( &sa, 0, sizeof(sa));

    sa.sa_handler = handle_sigint;

    /*
     * Register SIGINT handler so Ctrl+C can request a graceful shutdown.
     */
    sigaction( SIGINT, &sa, NULL);

    /*-----------------------------------------------------------------------
     * Create TCP socket
     *-----------------------------------------------------------------------*/

    int server_fd;

    struct sockaddr_in server_addr;

    server_fd = socket( AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1)
    {
        perror("socket");
        return 1;
    }

    printf("Server: socket created.\n");

    /*-----------------------------------------------------------------------
     * Configure server address
     *-----------------------------------------------------------------------*/

    memset( &server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;

    /*
     * INADDR_ANY allows the server to accept connections arriving
     * on any local network interface.
     */
    server_addr.sin_addr.s_addr = INADDR_ANY;

    /*
     * htons() converts the port number from host byte order
     * to network byte order.
     */
    server_addr.sin_port = htons(PORT);


    /*-----------------------------------------------------------------------
     * Bind socket to IP address and port
     *-----------------------------------------------------------------------*/
    if (bind( server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind");
        close(server_fd);
        return 1;
    }

    printf("Server: bind successful.\n");

    /*-----------------------------------------------------------------------
     * Start listening for clients
     *-----------------------------------------------------------------------*/
    if (listen(server_fd, 5) == -1)
    {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf( "Server: waiting for clients on port %d...\n", PORT);

    printf( "Main Thread ID: %lu\n", (unsigned long)pthread_self());

    /*-----------------------------------------------------------------------
     * Main server loop
     *-----------------------------------------------------------------------*/
    while (server_running)
    {
        int accepted_fd;

        /*
         * Wait for an incoming client connection.
         *
         * accept() returns a new socket descriptor for the connected
         * client. The original server_fd continues listening for
         * additional clients.
         */
        accepted_fd = accept( server_fd, NULL, NULL );


        /*
         * If Ctrl+C interrupts accept(), Linux can return EINTR.
         */
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

        /*-------------------------------------------------------------------
         * Allocate client context
         *------------------------------------------------------------------*/

        ClientContext *context =  malloc(sizeof(ClientContext));

        if (context == NULL)
        {
            perror("malloc");
            close(accepted_fd);
            continue;
        }

        context->client_fd = accepted_fd;

        context->equipment = &equipment;

        /*-------------------------------------------------------------------
         * Create worker thread
         *------------------------------------------------------------------*/
        pthread_t thread;

        if (pthread_create( &thread, NULL, client_handler, context) != 0)
        {
            perror("pthread_create");
            close(accepted_fd);
            free(context);
            continue;
        }
       
        /*
         * Detach the worker thread.
         *
         * A detached thread releases its thread resources automatically
         * when it terminates. The main thread does not wait for it using
         * pthread_join().
         */
        pthread_detach(thread);

    }


    /*-----------------------------------------------------------------------
     * Server shutdown
     *-----------------------------------------------------------------------*/
    printf("Server: shutting down...\n");

    close(server_fd);

    printf("Server: server socket closed.\n");

    /*
     * Release equipment resources, including its mutex.
     */
    equipment_destroy(&equipment);

    return 0;
}