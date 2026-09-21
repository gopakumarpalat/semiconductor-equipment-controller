#include <stdio.h>
#include <fcntl.h>
#include <mqueue.h>
#include <string.h>

#define QUEUE_NAME "/equipment_queue"

int main(void)
{
    mqd_t queue;

    char *commands[] =
    {
        "START ETCH01",
        "STOP ETCH01",
        "RESET ETCH01"
    };

    queue = mq_open( QUEUE_NAME, O_CREAT | O_WRONLY, 0666, NULL);

    if (queue == (mqd_t)-1)
    {
        perror("mq_open");
        return 1;
    }

    for (int i = 0; i < 3; i++)
    {
        printf( "Host: sending command: %s\n", commands[i]);

        // Last parameter of mq_send() is priority. mq_receive() will take the message based on highest priority.
        if (mq_send( queue, commands[i], strlen(commands[i]) + 1, i) == -1)
        {
            perror("mq_send");
            mq_close(queue);
            return 1;
        }
    }

    printf("Host: all commands sent.\n");

    mq_close(queue);

    return 0;
}