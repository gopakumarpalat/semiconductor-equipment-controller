#include <stdio.h>
#include <fcntl.h>
#include <mqueue.h>

#define QUEUE_NAME "/equipment_queue"

int main(void)
{
    mqd_t queue;
    char buffer[8192];

    queue = mq_open( QUEUE_NAME, O_RDONLY);

    if (queue == (mqd_t)-1)
    {
        perror("mq_open");
        return 1;
    }

    printf( "Equipment Controller: waiting for command...\n");

    for (int i = 0; i < 3; i++)
    {
        if (mq_receive( queue, buffer, sizeof(buffer), NULL) == -1)
        {
            perror("mq_receive");
            mq_close(queue);
            return 1;
        }

        printf( "Equipment Controller: received: %s\n", buffer);
    }

    mq_close(queue);

    mq_unlink(QUEUE_NAME);

    return 0;
}