#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 5000
#define SERVER_IP "127.0.0.1"

int main(void)
{
    int client_fd;

    struct sockaddr_in server_addr; // Which server to connect 

    char message[] = "START ETCH01";

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

    printf("Client: sending command: %s\n", message);

    if (send( client_fd, message, strlen(message) + 1, 0) == -1)
    {
        perror("send");
        close(client_fd);
        return 1;
    }

    printf("Client: command sent.\n");

    char buffer[1024];

    int bytes_received;

    bytes_received = recv( client_fd, buffer, sizeof(buffer), 0);

    if (bytes_received == -1)
    {
        perror("recv");
        close(client_fd);
        return 1;
    }
    else if (bytes_received == 0)
    {
        printf("Client: server closed connection.\n");
    }
    else
    {
        printf(
            "Client: received response: %s\n",
            buffer);
    }

    close(client_fd);

    return 0;
}