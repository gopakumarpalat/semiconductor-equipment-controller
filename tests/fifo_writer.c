#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(void)
{
    int fd;

    char command[] = "START ETCH01";

    fd = open("/tmp/equipment_fifo", O_WRONLY);

    printf( "Host: sending command: %s\n", command);

    write( fd, command, strlen(command) + 1);

    close(fd);

    return 0;
}