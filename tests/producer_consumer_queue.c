/**
 * @file producer_consumer_queue.c
 * @brief Producer-consumer task queue using pthread mutex and condition variable.
 *
 * This program demonstrates a thread-safe producer-consumer architecture
 * using:
 *
 *   - POSIX threads (pthread)
 *   - Mutex
 *   - Condition variable
 *   - Circular queue
 *   - Multiple worker threads
 *   - Graceful worker shutdown
 *
 * Architecture:
 *
 *              Producer Thread
 *                    |
 *                    | enqueue task
 *                    v
 *              +-----------+
 *              | Task Queue |
 *              +-----------+
 *                    |
 *              mutex + condition
 *                    |
 *          +---------+---------+
 *          |         |         |
 *          v         v         v
 *       Worker 1  Worker 2  Worker 3
 *          |         |         |
 *          +---------+---------+
 *                    |
 *                    v
 *              Process Task
 *
 * The producer adds tasks to the queue.
 * Workers wait when the queue is empty.
 * When a new task is added, the producer signals a waiting worker.
 *
 * A circular queue is used so that queue storage can be reused after
 * the head or tail reaches the end of the array.
 *
 * Important concepts demonstrated:
 *
 *   head  -> index of the next task to remove
 *   tail  -> index where the next task will be inserted
 *   count -> number of valid tasks currently in the queue
 *
 * The program also demonstrates graceful shutdown:
 *
 *   1. Producer finishes adding tasks.
 *   2. Main thread sets shutdown = 1.
 *   3. Main broadcasts the condition variable.
 *   4. Waiting workers wake up.
 *   5. Workers finish remaining tasks.
 *   6. Workers exit when the queue becomes empty.
 *
 * This pattern is commonly used in equipment-control software where
 * commands received from a communication layer are placed into a queue
 * and processed asynchronously by worker threads.
 */

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

/* -------------------------------------------------------------------------
 * Configuration
 * ------------------------------------------------------------------------- */

/**
 * Maximum number of tasks that can be stored in the queue.
 */
#define QUEUE_SIZE 10

/**
 * Number of worker threads used to process tasks.
 */
#define WORKER_COUNT 3


/* -------------------------------------------------------------------------
 * Task definition
 * ------------------------------------------------------------------------- */

/**
 * @brief Represents one task/command in the queue.
 *
 * In a real equipment-control application, this could represent commands
 * such as:
 *
 *   START
 *   STOP
 *   STATUS
 *   RESET
 *   ALARM
 */
typedef struct
{
    int id;
    char command[32];

} Task;


/* -------------------------------------------------------------------------
 * Task Queue definition
 * ------------------------------------------------------------------------- */

/**
 * @brief Thread-safe circular task queue.
 *
 * The queue is implemented using a fixed-size array.
 *
 * head:
 *     Points to the next task that should be removed.
 *
 * tail:
 *     Points to the next free position where a new task should be added.
 *
 * count:
 *     Number of valid tasks currently stored in the queue.
 *
 * shutdown:
 *     Set to 1 when the application wants workers to stop.
 *
 * mutex:
 *     Protects all shared queue data.
 *
 * condition:
 *     Used by workers to sleep when the queue is empty and
 *     by the producer/main thread to wake waiting workers.
 */
typedef struct
{
    Task tasks[QUEUE_SIZE];

    int head;
    int tail;
    int count;

    int shutdown;

    pthread_mutex_t mutex;
    pthread_cond_t condition;

} TaskQueue;



/* -------------------------------------------------------------------------
 * Global Queue
 * ------------------------------------------------------------------------- */

/**
 * @brief Shared task queue.
 *
 * Initial state:
 *
 *     head     = 0
 *     tail     = 0
 *     count    = 0
 *     shutdown = 0
 *
 * Both producer and worker threads access this queue, so all access
 * to the shared queue state must be protected by queue.mutex.
 */
TaskQueue queue =
{
    .head = 0,
    .tail = 0,
    .count = 0,
    .shutdown = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .condition = PTHREAD_COND_INITIALIZER
};


/* -------------------------------------------------------------------------
 * Function: enqueue_task
 * ------------------------------------------------------------------------- */

/**
 * @brief Add a task to the shared queue.
 *
 * The function:
 *
 *   1. Locks the queue mutex.
 *   2. Checks whether the queue has space.
 *   3. Stores the task at the current tail position.
 *   4. Moves tail to the next position.
 *   5. Increments count.
 *   6. Signals one waiting worker.
 *   7. Unlocks the mutex.
 *
 * Circular queue logic:
 *
 *     tail = (tail + 1) % QUEUE_SIZE;
 *
 * Example with QUEUE_SIZE = 10:
 *
 *     0 -> 1 -> 2 -> ... -> 8 -> 9 -> 0 -> 1 ...
 *
 * The modulo operation makes the queue wrap around to index 0
 * after reaching the last array element.
 *
 * @param id      Task identifier.
 * @param command Command string to add to the queue.
 */
void enqueue_task(int id, const char *command)
{
    pthread_mutex_lock(&queue.mutex);

    /*
     * Make sure there is space before adding a task.
     *
     * count == QUEUE_SIZE means the queue is full.
     */
    if (queue.count < QUEUE_SIZE)
    {
         /*
         * Store the task at the current tail position.
         */
        queue.tasks[queue.tail].id = id;

        strncpy(queue.tasks[queue.tail].command, command, sizeof(queue.tasks[queue.tail].command) - 1 );

        /*
         * Explicitly terminate the string.
         *
         * This protects against missing '\0' termination when
         * the source string is longer than the destination buffer.
         */
        queue.tasks[queue.tail].command[sizeof(queue.tasks[queue.tail].command) - 1] = '\0';

        /*
         * Move tail to the next position.
         *
         * This is what makes the queue circular.
         */
        queue.tail = (queue.tail + 1) % QUEUE_SIZE;

        /*
         * One new task has been added.
         */
        queue.count++;

        printf("Producer: Added task %d (%s), queue count = %d\n", id, command, queue.count);

        /*
         * A new task is available.
         *
         * Wake ONE worker that may currently be waiting because
         * the queue was empty.
         *
         * We use signal() rather than broadcast() because one new
         * task requires only one worker.
         */
        pthread_cond_signal(&queue.condition);
    }

    pthread_mutex_unlock(&queue.mutex);
}


/* -------------------------------------------------------------------------
 * Function: producer
 * ------------------------------------------------------------------------- */

/**
 * @brief Producer thread function.
 *
 * The producer simulates commands arriving from an external source,
 * such as:
 *
 *     TCP server
 *     Equipment communication interface
 *     Host system
 *
 * Each command is added to the shared task queue.
 *
 * @param arg Thread argument. Not used in this example.
 *
 * @return NULL
 */
void *producer(void *arg)
{
    (void)arg;

    /*
     * Commands that simulate equipment commands.
     */
    const char *commands[] =
    {
        "START",
        "STATUS",
        "ALARM",
        "STOP",
        "RESET",
        "STATUS"
    };

    int command_count = sizeof(commands) / sizeof(commands[0]);

    printf("Producer: Started.\n");

    /*
     * Produce one task at a time.
     *
     * sleep(1) is only for demonstration so that we can observe
     * producer and worker interaction clearly.
     */
    for (int i = 0; i < command_count; i++)
    {
        sleep(1);

        enqueue_task(i + 1, commands[i]);
    }

    printf("Producer: Finished producing tasks.\n");

    return NULL;
}



/* -------------------------------------------------------------------------
 * Function: worker
 * ------------------------------------------------------------------------- */

/**
 * @brief Worker thread function.
 *
 * Each worker continuously:
 *
 *     1. Locks the queue.
 *     2. Waits if the queue is empty.
 *     3. Checks for shutdown.
 *     4. Removes one task.
 *     5. Updates head and count.
 *     6. Unlocks the queue.
 *     7. Processes the task.
 *
 * Important:
 *
 * The mutex is released BEFORE processing the task.
 *
 * This is intentional.
 *
 * We only need the mutex while accessing shared queue data.
 * Processing the task while holding the mutex would prevent other
 * workers from accessing the queue and reduce concurrency.
 *
 * @param arg Pointer to the worker ID.
 *
 * @return NULL
 */
void *worker(void *arg)
{
    /*
     * Convert the generic pthread argument back to int pointer.
     */
    int worker_id = *(int *)arg;

    printf( "Worker %d: Started.\n", worker_id );

    while (1)
    {
        pthread_mutex_lock(&queue.mutex);

        /*
         * Wait while:
         *
         *   1. There are no tasks in the queue.
         *   2. Shutdown has not been requested.
         *
         * pthread_cond_wait() does two important things:
         *
         *   - releases the mutex while waiting
         *   - reacquires the mutex before returning
         *
         * The condition is checked using while(), not if().
         * After waking up, the shared state must always be checked again.
         */
        while (queue.count == 0 && !queue.shutdown)
        {
            printf("Worker %d: Queue empty. Waiting...\n", worker_id);

            pthread_cond_wait(&queue.condition, &queue.mutex);
        }

        /*
         * If shutdown was requested AND there are no remaining tasks,
         * this worker can safely exit.
         *
         * If tasks are still available, the worker must process them
         * before shutting down.
         */
        if (queue.shutdown && queue.count == 0)
        {
            pthread_mutex_unlock(&queue.mutex);

            printf("Worker %d: Shutting down.\n", worker_id);

            break;
        }

        /*
         * Remove the task at head.
         *
         * head always points to the oldest task in the queue.
         *
         * This gives us FIFO behavior:
         *
         *     First In -> First Out
         */
        Task task = queue.tasks[queue.head];

        /*
         * Move head to the next queue position.
         *
         * Again, modulo makes the queue circular.
         */
        queue.head = (queue.head + 1) % QUEUE_SIZE;

        /*
         * One task has been removed.
         */
        queue.count--;

        printf("Worker %d: Got task %d (%s), queue count = %d\n", worker_id, task.id, task.command, queue.count);

        /*
         * We have finished accessing shared queue data.
         *
         * Release the mutex BEFORE processing the task.
         */
        pthread_mutex_unlock(&queue.mutex);

        /*
         * Process the task outside the critical section.
         *
         * In a real application this could be:
         *
         *     equipment_start()
         *     equipment_stop()
         *     equipment_reset()
         *     equipment_status()
         *
         * Keeping this outside the mutex allows other workers
         * to access the queue while this worker is busy.
         */
        printf( "Worker %d: Processing %s...\n", worker_id, task.command);

        /*
         * Simulate task processing time.
         */
        sleep(2);

        printf( "Worker %d: Completed task %d (%s)\n", worker_id, task.id, task.command);
    }

    return NULL;
}



/* -------------------------------------------------------------------------
 * Function: main
 * ------------------------------------------------------------------------- */

/**
 * @brief Program entry point.
 *
 * Creates:
 *
 *     - multiple worker threads
 *     - one producer thread
 *
 * After the producer finishes, main requests a graceful shutdown.
 *
 * Shutdown sequence:
 *
 *     Producer finishes
 *          |
 *          v
 *     shutdown = 1
 *          |
 *          v
 *     pthread_cond_broadcast()
 *          |
 *          v
 *     Wake all waiting workers
 *          |
 *          v
 *     Workers process remaining tasks
 *          |
 *          v
 *     Queue becomes empty
 *          |
 *          v
 *     Workers exit
 */
int main(void)
{
    pthread_t producer_thread;
    pthread_t workers[WORKER_COUNT];

    int worker_ids[WORKER_COUNT];

    printf("Main: Starting producer-consumer system.\n");

    /*
     * Create worker threads.
     *
     * Each worker receives its own worker ID.
     */
    for (int i = 0; i < WORKER_COUNT; i++)
    {
        worker_ids[i] = i + 1;

        pthread_create(&workers[i], NULL, worker, &worker_ids[i] );
    }

    /*
     * Create the producer thread.
     */
    pthread_create(&producer_thread, NULL, producer, NULL);

    /*
     * Wait until the producer has finished adding all tasks.
     */
    pthread_join(producer_thread, NULL);

    printf("Main: Producer finished.\n");

    /*
     * Request graceful shutdown.
     *
     * Workers should NOT immediately exit.
     *
     * They must first process all remaining tasks in the queue.
     */
    pthread_mutex_lock(&queue.mutex);

    queue.shutdown = 1;

    /*
     * Wake ALL workers.
     *
     * Some workers may currently be sleeping inside
     * pthread_cond_wait().
     *
     * Since shutdown affects every worker, broadcast()
     * is appropriate here.
     */
    pthread_cond_broadcast(&queue.condition);

    pthread_mutex_unlock(&queue.mutex);

    /*
     * Wait for all workers to finish.
     */
    for (int i = 0; i < WORKER_COUNT; i++)
    {
        pthread_join(workers[i], NULL);
    }

    pthread_mutex_destroy(&queue.mutex);

    pthread_cond_destroy(&queue.condition);

    printf("Main: All workers stopped.\n");

    printf("Program completed.\n");

    return 0;
}