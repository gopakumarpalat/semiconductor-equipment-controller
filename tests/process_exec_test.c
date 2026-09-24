#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void)
{
    pid_t pid;
    int status;

    printf("Parent: PID = %d\n", getpid());

    pid = fork();

    if (pid < 0)
    {
        perror("fork failed");
        return 1;
    }

    if (pid == 0)
    {
        /*
         * Child process
         */
        printf("Child: PID = %d\n", getpid());
        printf("Child: Executing ls...\n");

        execl( "/bin/ls", "ls", "-l", NULL);

        /*
         * This line executes only if exec fails.
         */
        perror("exec failed");
        exit(1);
    }

    /*
     * Parent process
     */
    printf( "Parent: Created child with PID = %d\n", pid);

    printf("Parent: Waiting for child...\n");

    waitpid(pid, &status, 0);

    printf("Parent: Child finished.\n");

    return 0;
}