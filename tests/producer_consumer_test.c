/*

Producer Thread
      │
      │ add command
      ▼
   Queue
      │
      │ remove command
      ▼
Consumer Thread

We will use following for implementation
1. Mutex - To protect Queue from concurrent handling.
2. Condition variable - If Queue is empty consumer will wait.


Final result :
------------------------------------------------------------------
    Main
    │
    ├── create Consumer
    │
    └── create Producer

    Consumer:

        Consumer
           ↓
        Queue empty
           ↓
        WAIT 😴

    Producer:
        Producer
            ↓
          START
          STOP
          RESET
          START
          STOP
            ↓
        signal condition

    Consumer:
        WAKE UP 🔔
            ↓
        take START
            ↓
        unlock mutex
            ↓
        process START
            ↓
        take STOP
            ↓
        process STOP
            ↓
           ...
            ↓
        queue empty
            ↓
          WAIT 😴


    Producer finished.

    Main:

        shutdown_requested = 1
                  ↓
         condition signal 🔔

    Consumer wakes:

        shutdown_requested == 1
                  ↓
                break
                  ↓
        Consumer: shutting down.
                  ↓
               return

    Main:

        pthread_join(consumer)
                  ↓
        Main: all threads finished.
                  ↓
            destroy mutex
            destroy condition
                  ↓
                 EXIT

*/

#include <stdio.h>
#include <pthread.h>
#include <windows.h>

typedef enum
{
    CMD_START,
    CMD_STOP,
    CMD_RESET
} Command;

#define QUEUE_SIZE 5

int queue[QUEUE_SIZE];

int head = 0;
int tail = 0;
int count = 0; //Queue item count

int shutdown_requested = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;

const char* command_to_string(Command command)
{
    switch (command)
    {
        case CMD_START:
            return "START";

        case CMD_STOP:
            return "STOP";

        case CMD_RESET:
            return "RESET";
        
        default:
            break;
    }
}

void queue_push(Command command)
{
    if (count == QUEUE_SIZE)
    {
        printf("Queue is full!\n");
        return;
    }

    queue[tail] = command;
    tail = (tail + 1) % QUEUE_SIZE;
    count++;

    printf("Producer: added command %s.\n", command_to_string(command));
    //printf("Producer: tail = %d, count = %d\n", tail, count);
}


Command queue_pop()
{
    if (count == 0)
    {
        printf("Queue is empty!\n");
        return CMD_RESET;
    }

    Command command = queue[head];
    head = (head + 1) % QUEUE_SIZE;
    count--;
    return command;
}

void *producer( void *arg)
{
    printf("Producer: started.\n");

    pthread_mutex_lock(&mutex);

    if (count < QUEUE_SIZE)
    {
        queue_push(CMD_START);
        queue_push(CMD_STOP);
        queue_push(CMD_RESET);
        queue_push(CMD_START);
        queue_push(CMD_STOP);
    }

    pthread_cond_signal(&condition);

    pthread_mutex_unlock(&mutex);

    return NULL;
}

/* Consumer */
void *consumer(void *arg)
{
    printf("Consumer: started.\n");

    while (1)
    {
        pthread_mutex_lock(&mutex);

        while (count == 0 && !shutdown_requested)
        {
            printf("Consumer: queue is empty. Waiting...\n");

            pthread_cond_wait(&condition, &mutex);
        }

        /*
         * If shutdown was requested, exit the thread.
         */
        if (shutdown_requested)
        {
            pthread_mutex_unlock(&mutex);
            break;
        }

        /*
         * Take ONE command from the queue.
         */
        Command command = queue_pop();

        printf( "Consumer: received command %s.\n", command_to_string(command));

        pthread_mutex_unlock(&mutex);

        /*
         * Process command without holding the mutex.
         */
        printf( "Consumer: processing command %s...\n", command_to_string(command));

        Sleep(1000);
    }

    /*printf("Queue Items\n");
    for(int i=0; i<QUEUE_SIZE; i++)
    {
         printf("queue[%d] = %d\n", i, queue[i]);
    }*/

    printf("Consumer: shutting down.\n");

    return NULL;
}



int main()
{
    pthread_t producer_thread;
    pthread_t consumer_thread;

    printf("main(): starting threads...\n");

    pthread_create( &consumer_thread, NULL, consumer, NULL);

    Sleep(1000);

    pthread_create(&producer_thread, NULL, producer, NULL);

    pthread_join( producer_thread, NULL);

    /* Request consumer shutdown */
    pthread_mutex_lock(&mutex);
    shutdown_requested = 1;
    pthread_cond_signal(&condition);
    pthread_mutex_unlock(&mutex);

    /* Wait for consumer to finish */
    pthread_join( consumer_thread, NULL);

    printf("Main: all threads finished.\n");

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condition);

    return 0;
}

