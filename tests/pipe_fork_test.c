#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

int main(void)
{
    int fd[2];

    char message[] = "START ETCH01";
    char buffer[100];

    pid_t pid;

    /* Create pipe */
    pipe(fd);

    /* Create child process */
    pid = fork();

     if (pid > 0)
    {
        /* Parent */

        close(fd[0]);

        printf("Parent: sending command...\n");

        write(fd[1], message, strlen(message) + 1);

        close(fd[1]);
    }
    else if (pid == 0)
    {
        /* Child */

        close(fd[1]);

        read(fd[0], buffer, sizeof(buffer));

        printf("Child: received command: %s\n", buffer);

        close(fd[0]);
    }
    else
    {
        printf("fork() failed.\n");
    }

    return 0;
}