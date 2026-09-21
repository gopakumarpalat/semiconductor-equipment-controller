#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

int main(void)
{
    int parent_to_child[2];
    int child_to_parent[2];

    char command[] = "START ETCH01";
    char response[] = "ACK - Command received";

    char buffer[100];

    pid_t pid;

    /* Create two pipes */
    pipe(parent_to_child);
    pipe(child_to_parent);

    /* Create child */
    pid = fork();

    if (pid > 0)
    {
        /* =========================
           Parent
           ========================= */

        close(parent_to_child[0]);
        close(child_to_parent[1]);

        printf("Parent: sending command: %s\n", command);

        write(
            parent_to_child[1],
            command,
            strlen(command) + 1);

        read(
            child_to_parent[0],
            buffer,
            sizeof(buffer));

        printf(
            "Parent: received response: %s\n",
            buffer);

        close(parent_to_child[1]);
        close(child_to_parent[0]);
    }
    else if (pid == 0)
    {
        /* =========================
           Child
           ========================= */

        close(parent_to_child[1]);
        close(child_to_parent[0]);

        read(
            parent_to_child[0],
            buffer,
            sizeof(buffer));

        printf(
            "Child: received command: %s\n",
            buffer);

        printf(
            "Child: sending response: %s\n",
            response);

        write(
            child_to_parent[1],
            response,
            strlen(response) + 1);

        close(parent_to_child[0]);
        close(child_to_parent[1]);
    }
    else
    {
        printf("fork() failed.\n");
    }

    return 0;
}