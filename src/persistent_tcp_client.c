/**
 * @file persistent_tcp_client.c
 * @brief TCP client for communicating with the semiconductor equipment server.
 *
 * @details
 * This program implements a persistent TCP client that connects to the
 * equipment control server and sends a sequence of equipment commands.
 *
 * The client maintains a single TCP connection while sending multiple
 * commands and receiving a response for each command.
 *
 * Supported commands in this test client:
 * - START
 * - STATUS
 * - ALARM
 * - STOP
 * - RESET
 *
 * Communication flow:
 *
 *     Client                          Server
 *       |                               |
 *       |-------- connect() ----------->|
 *       |                               |
 *       |-------- command ------------->|
 *       |<------- response -------------|
 *       |                               |
 *       |-------- command ------------->|
 *       |<------- response -------------|
 *       |                               |
 *       |-------- close() ------------->|
 *
 * @note This implementation uses POSIX socket APIs and is intended
 *       to be built and executed in a Linux/WSL environment.
 *
 * @author Gopakumar Palat
 * @date 2026-09-18
 */

 /* ================================================================
 * TCP Communication Flow
 * ================================================================ */

/*
 * Client                                      Server
 *   │                                           │
 *   │──────── connect() ──────────────────────►│
 *   │                                           │
 *   │──────── START ──────────────────────────►│
 *   │◄──────── ACK START ──────────────────────│
 *   │                                           │
 *   │──────── STATUS ─────────────────────────►│
 *   │◄──────── STATE RUNNING ──────────────────│
 *   │                                           │
 *   │──────── ALARM ──────────────────────────►│
 *   │◄──────── ACK ALARM ──────────────────────│
 *   │                                           │
 *   │──────── STOP ───────────────────────────►│
 *   │◄──────── ACK STOP ───────────────────────│
 *   │                                           │
 *   │──────── STATUS ─────────────────────────►│
 *   │◄──────── STATE READY ────────────────────│
 *   │                                           │
 *   │──────── RESET ──────────────────────────►│
 *   │◄──────── ACK RESET ──────────────────────│
 *   │                                           │
 *   │──────── STATUS ─────────────────────────►│
 *   │◄──────── STATE IDLE ─────────────────────│
 *   │                                           │
 *   │──────── close() ────────────────────────►│
 *   │                                           │
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "equipment_config.h"


/* ================================================================
 * Private Functions
 * ================================================================ */

/**
 * @brief Receive a complete line from the TCP server.
 *
 * TCP does not preserve message boundaries. Therefore, this function
 * receives data one byte at a time until a newline character is
 * received.
 *
 * The newline character is not stored in the output buffer.
 * The received data is always null-terminated.
 *
 * @param client_fd   TCP socket file descriptor.
 * @param buffer      Buffer used to store the received line.
 * @param buffer_size Size of the receive buffer.
 *
 * @return Number of characters received, excluding the newline.
 * @return 0 if the server closed the connection.
 * @return -1 if a receive error occurred.
 */
int recv_line( int client_fd, char *buffer, int buffer_size)
{
    int total = 0;

    while (total < buffer_size - 1)
    {
        char ch;

        /*
         * Receive one byte from the server.
         */
        int bytes_received = recv( client_fd, &ch, 1, 0);

        /*
         * recv() returned an error.
         */
        if (bytes_received == -1)
        {
            return -1;
        }

         /*
         * recv() returned 0, which means the server
         * has closed the connection.
         */
        if (bytes_received == 0)
        {
            return 0;
        }

        /*
         * Newline marks the end of the message.
         */
        if (ch == '\n')
        {
            buffer[total] = '\0';
            return total;
        }

        buffer[total] = ch;
        total++;
    }

    /*
     * Buffer is full. Ensure null termination.
     */
    buffer[total] = '\0';

    return total;
}


/* ================================================================
 * Main Function
 * ================================================================ */

/**
 * @brief Entry point of the persistent TCP client.
 *
 * Creates a TCP socket, connects to the equipment server, sends
 * a predefined sequence of commands, receives responses, and
 * finally closes the connection.
 *
 * @return 0 on successful completion.
 * @return 1 if a socket, connection, send, or receive error occurs.
 */
int main(void)
{
    /*-----------------------------------------------------------------------
     * Read PORT from config file
     *-----------------------------------------------------------------------*/
    EquipmentConfig config = {0};

    if (equipment_config_load( "config/equipment.conf", &config) != 0)
    {
        printf("Failed to load equipment configuration.\n");
        return 1;
    }

    if (equipment_config_validate(&config) != 0)
    {
        printf("Equipment configuration validation failed.\n");
        return 1;
    }


    int client_fd;

    /*
     * Structure containing the server's network address.
     */
    struct sockaddr_in server_addr;

    /*
     * Commands sent to the equipment server.
     *
     * Each command ends with '\n' because the server uses
     * newline-based message framing.
     */
    char *commands[] =
    {
        "START\n",
        "STATUS\n",
        "ALARM\n",
        "STOP\n",
        "STATUS\n",
        "RESET\n",
        "STATUS\n"
    };

    /* ============================================================
     * Create TCP Socket
     * ============================================================ */
    client_fd = socket( AF_INET, SOCK_STREAM, 0);

    if (client_fd == -1)
    {
        perror("socket");
        return 1;
    }

    printf("Client: socket created.\n");

    /* ============================================================
     * Configure Server Address
     * ============================================================ */

    /*
     * Clear the server address structure before assigning values.
     */
    memset( &server_addr, 0, sizeof(server_addr));

    /*
     * Specify IPv4 address family.
     */
    server_addr.sin_family = AF_INET;

    /*
     * Convert the port number from host byte order to
     * network byte order.
     */
    server_addr.sin_port = htons(config.tcp_port);

    /*
     * Convert the server IP address from text format
     * to binary network format.
     *
     * The converted address is stored in sin_addr.
     */
    if (inet_pton( AF_INET, config.server_ip, &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton");
        close(client_fd);
        return 1;
    }

    /* ============================================================
     * Connect to Server
     * ============================================================ */
    if (connect( client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("connect");
        close(client_fd);
        return 1;
    }

    printf("Client: connected to server.\n");


    /* ============================================================
     * Send Commands and Receive Responses
     * ============================================================ */
    for (int i = 0; i < 7; i++)
    {
        printf( "Client: sending command: %s", commands[i]);

        /*
         * Send the complete command to the server.
         */
        if (send( client_fd, commands[i], strlen(commands[i]), 0) == -1)
        {
            perror("send");
            close(client_fd);
            return 1;
        }

        printf("Client: command sent.\n");

        /*
         * Buffer used to store the server response.
         */
        char buffer[1024];

        int bytes_received;

        /*
         * Receive one complete line from the server.
         */
        bytes_received = recv_line( client_fd, buffer, sizeof(buffer));

        /*
         * Receive error.
         */
        if (bytes_received == -1)
        {
            perror("recv_line");
            close(client_fd);
            return 1;
        }

        /*
         * Server closed the connection.
         */
        if (bytes_received == 0)
        {
            printf("Client: server disconnected.\n");
            break;
        }

        printf( "Client: received response: %s\n", buffer);
    }

    /* ============================================================
     * Close Connection
     * ============================================================ */

    close(client_fd);

    return 0;
}