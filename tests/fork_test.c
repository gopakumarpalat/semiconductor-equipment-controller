#include <stdio.h>
#include <unistd.h>

int main(void)
{
    pid_t pid;

    pid = fork();

    printf("PID : %d\n", pid);

    if (pid > 0)
    {
        printf("I am the Parent process.\n");
    }
    else if (pid == 0)
    {
        printf("I am the Child process.\n");
        printf("Child PID : %d\n", getpid());
        printf("Parent PID: %d\n", getppid());
    }
    else
    {
        printf("fork() failed.\n");
    }

    return 0;
}