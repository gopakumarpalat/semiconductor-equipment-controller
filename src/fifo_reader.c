#include<stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(void)
{
    int fd;
    char buffer[100];

    printf("Equipment Controller: waiting for command...\n");

    fd = open("/tmp/equipment_fifo", O_RDONLY);

    read(fd, buffer, sizeof(buffer));

    printf( "Equipment Controller: received command: %s\n", buffer);

    close(fd);

    return 0;
}