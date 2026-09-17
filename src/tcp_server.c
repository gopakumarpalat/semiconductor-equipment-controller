#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 5000

int main(void)
{
    int server_fd;
    int client_fd;

    struct sockaddr_in server_addr;

    char buffer[1024];

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

    printf("Server: waiting for client on port %d...\n", PORT);

    while(1)
    {

        client_fd = accept( server_fd, NULL, NULL);

        if (client_fd == -1)
        {
            perror("accept");
            continue;
        }

        printf("Server: client connected.\n");

        int bytes_received;
        
        /*
        * return value of recv = no of bytes received;
        *                        positive → data received
        *                        0        → connection closed
        *                        -1       → error
        */
        bytes_received = recv( client_fd, buffer, sizeof(buffer), 0);

        if (bytes_received == -1)
        {
            perror("recv");
            close(client_fd);
            continue;
        }

        if (bytes_received == 0)
        {
            printf("Server: client disconnected.\n");
            close(client_fd);
            continue;
        }

        printf( "Server: received command: %s\n", buffer);

        char response[] = "ACK START";

        printf( "Server: sending response: %s\n", response);

        if (send( client_fd, response, strlen(response) + 1, 0) == -1)
        {
            perror("send");
            close(client_fd);
            continue;
        }

        printf("Server: response sent.\n");

        close(client_fd);

        printf("Server: waiting for next client...\n");

    }

    close(server_fd);

    return 0;
}