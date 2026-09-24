#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main(void)
{
    pid_t pid;

    pid = fork();

    if (pid < 0)
    {
        perror("fork failed");
        return 1;
    }

    if (pid == 0)
    {
        printf("Child: executing ls\n");
        printf("Child PID : %d\n", getpid());

        execl("/bin/ls", "ls", "-l", NULL);

        perror("exec failed");
        return 1;
    }

    if(pid > 0)
    {
        printf("Parent PID : %d\n", getpid());
    }

    printf("Parent: child created\n");

    return 0;
}