#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define QUEUE_SIZE 8
#define WORKER_COUNT 2


typedef enum
{
    CMD_START,
    CMD_STOP,
    CMD_STATUS

} CommandType;


typedef struct
{
    CommandType type;

    char equipment_name[32];

} Command;


typedef struct
{
    Command commands[QUEUE_SIZE];

    int head;
    int tail;
    int count;

    /*
     * Indicates that no more commands
     * will be added.
     */
    int shutdown;

    pthread_mutex_t mutex;

    pthread_cond_t condition;

} CommandQueue;


typedef struct
{
    CommandQueue *queue;

    int worker_id;

} WorkerContext;


/**
 * @brief Initialize command queue.
 */
void queue_init(CommandQueue *queue)
{
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
    queue->shutdown = 0;

    pthread_mutex_init(     &queue->mutex,
        NULL
    );

    pthread_cond_init(
        &queue->condition,
        NULL
    );
}


/**
 * @brief Add command to queue.
 */
int queue_push(
    CommandQueue *queue,
    Command command
)
{
    pthread_mutex_lock(
        &queue->mutex
    );


    /*
     * Queue is full.
     */
    if (queue->count == QUEUE_SIZE)
    {
        pthread_mutex_unlock(
            &queue->mutex
        );

        return -1;
    }


    /*
     * Add command.
     */
    queue->commands[queue->tail] = command;


    queue->tail =
        (queue->tail + 1) % QUEUE_SIZE;


    queue->count++;


    /*
     * Tell one waiting worker that
     * a command is available.
     */
    pthread_cond_signal(
        &queue->condition
    );


    pthread_mutex_unlock(
        &queue->mutex
    );


    return 0;
}


/**
 * @brief Convert command to text.
 */
const char *command_to_string(
    CommandType type
)
{
    switch (type)
    {
        case CMD_START:
            return "START";

        case CMD_STOP:
            return "STOP";

        case CMD_STATUS:
            return "STATUS";

        default:
            return "UNKNOWN";
    }
}


/**
 * @brief Producer thread.
 */
void *producer(void *arg)
{
    CommandQueue *queue =
        (CommandQueue *)arg;


    Command commands[] =
    {
        { CMD_START,  "ETCH01" },
        { CMD_STATUS, "CVD01"  },
        { CMD_STOP,   "ETCH01" },
        { CMD_START,  "CVD01"  }
    };


    int command_count =
        sizeof(commands) /
        sizeof(commands[0]);


    for (int i = 0; i < command_count; i++)
    {
        if (queue_push(
                queue,
                commands[i]) == 0)
        {
            printf(
                "Producer: Added %s for %s\n",
                command_to_string(
                    commands[i].type
                ),
                commands[i].equipment_name
            );
        }
    }


    /*
     * Producer has finished.
     */
    pthread_mutex_lock(
        &queue->mutex
    );


    queue->shutdown = 1;


    /*
     * Wake ALL workers.
     *
     * They need to check shutdown.
     */
    pthread_cond_broadcast(
        &queue->condition
    );


    pthread_mutex_unlock(
        &queue->mutex
    );


    printf(
        "Producer: Finished.\n"
    );


    return NULL;
}


/**
 * @brief Worker thread.
 */
void *worker(void *arg)
{
    WorkerContext *context =
        (WorkerContext *)arg;


    CommandQueue *queue =
        context->queue;


    int worker_id =
        context->worker_id;


    Command command;


    while (1)
    {
        /*
         * Lock queue.
         */
        pthread_mutex_lock(
            &queue->mutex
        );


        /*
         * Wait while:
         *
         *     queue is empty
         *
         * AND
         *
         *     producer has not finished.
         */
        while (
            queue->count == 0 &&
            !queue->shutdown
        )
        {
            printf(
                "Worker %d: Waiting...\n",
                worker_id
            );


            pthread_cond_wait(
                &queue->condition,
                &queue->mutex
            );
        }


        /*
         * If there are no commands left
         * and producer has finished,
         * this worker can exit.
         */
        if (
            queue->count == 0 &&
            queue->shutdown
        )
        {
            pthread_mutex_unlock(
                &queue->mutex
            );

            break;
        }


        /*
         * Get oldest command.
         */
        command =
            queue->commands[queue->head];


        queue->head =
            (queue->head + 1) %
            QUEUE_SIZE;


        queue->count--;


        pthread_mutex_unlock(
            &queue->mutex
        );


        /*
         * Process command OUTSIDE mutex.
         */
        printf(
            "Worker %d: Processing %s for %s\n",
            worker_id,
            command_to_string(
                command.type
            ),
            command.equipment_name
        );


        sleep(1);
    }


    printf(
        "Worker %d: Finished.\n",
        worker_id
    );


    return NULL;
}


/**
 * @brief Main function.
 */
int main(void)
{
    CommandQueue queue;


    WorkerContext contexts[
        WORKER_COUNT
    ];


    pthread_t producer_thread;


    pthread_t worker_threads[
        WORKER_COUNT
    ];


    queue_init(&queue);


    /*
     * Create producer.
     */
    pthread_create(
        &producer_thread,
        NULL,
        producer,
        &queue
    );


    /*
     * Create workers.
     */
    for (int i = 0;
         i < WORKER_COUNT;
         i++)
    {
        contexts[i].queue =
            &queue;


        contexts[i].worker_id =
            i + 1;


        pthread_create(
            &worker_threads[i],
            NULL,
            worker,
            &contexts[i]
        );
    }


    /*
     * Wait for producer.
     */
    pthread_join(
        producer_thread,
        NULL
    );


    /*
     * Wait for workers.
     */
    for (int i = 0;
         i < WORKER_COUNT;
         i++)
    {
        pthread_join(
            worker_threads[i],
            NULL
        );
    }


    /*
     * Destroy synchronization objects.
     */
    pthread_mutex_destroy(
        &queue.mutex
    );


    pthread_cond_destroy(
        &queue.condition
    );


    printf(
        "Program completed.\n"
    );


    return 0;
}