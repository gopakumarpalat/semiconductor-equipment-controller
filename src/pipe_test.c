#include<stdio.h>
#include <unistd.h>
#include <string.h>

int main()
{
    int fd[2];

    pipe(fd);




    char message[] = "Hello from the pipe!";
    char buffer[sizeof(message)];

    
    write(fd[1], message, strlen(message) + 1);

    read(fd[0], buffer, sizeof(message));

    printf("Received: %s\n", buffer);


    

    return 0;
}