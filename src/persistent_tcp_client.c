/*

Client                                      Server
  │                                           │
  │──────── connect() ──────────────────────►│
  │                                           │
  │──────── START ──────────────────────────►│
  │◄──────── ACK START ──────────────────────│
  │                                           │
  │──────── STATUS ─────────────────────────►│
  │◄──────── STATE RUNNING ──────────────────│
  │                                           │
  │──────── STOP ───────────────────────────►│
  │◄──────── ACK STOP ───────────────────────│
  │                                           │
  │──────── STATUS ─────────────────────────►│
  │◄──────── STATE READY ────────────────────│
  │                                           │
  │──────── RESET ──────────────────────────►│
  │◄──────── ACK RESET ──────────────────────│
  │                                           │
  │──────── STATUS ─────────────────────────►│
  │◄──────── STATE IDLE ─────────────────────│
  │                                           │
  │──────── close() ────────────────────────►│
  │                                           │

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 5000
#define SERVER_IP "127.0.0.1"


int recv_line( int client_fd, char *buffer, int buffer_size)
{
    int total = 0;

    while (total < buffer_size - 1)
    {
        char ch;

        int bytes_received = recv(
            client_fd,
            &ch,
            1,
            0);

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



int main(void)
{
    int client_fd;

    struct sockaddr_in server_addr; // Which server to connect 

    //char *commands[] = { "START ETCH01\n", "STOP ETCH01\n", "RESET ETCH01\n" };

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

    // Create client socket
    client_fd = socket( AF_INET, SOCK_STREAM, 0);

    if (client_fd == -1)
    {
        perror("socket");
        return 1;
    }

    printf("Client: socket created.\n");

    // set 0 to server_addr
    memset( &server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Convert IP address string to binary network format.
    // Converted IP adress will store into &server_addr.sin_addr.
    if (inet_pton( AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton");
        close(client_fd);
        return 1;
    }

    // Connect to the server
    if (connect( client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("connect");
        close(client_fd);
        return 1;
    }

    printf("Client: connected to server.\n");

    for (int i = 0; i < 7; i++)
    {
        printf( "Client: sending command: %s", commands[i]);

        if (send( client_fd, commands[i], strlen(commands[i]), 0) == -1)
        {
            perror("send");
            close(client_fd);
            return 1;
        }

        printf("Client: command sent.\n");

        char buffer[1024];

        int bytes_received;

        bytes_received = recv_line( client_fd, buffer, sizeof(buffer));

        if (bytes_received == -1)
        {
            perror("recv_line");
            close(client_fd);
            return 1;
        }

        if (bytes_received == 0)
        {
            printf("Client: server disconnected.\n");
            break;
        }

        printf( "Client: received response: %s\n", buffer);
    }

    close(client_fd);

    return 0;
}