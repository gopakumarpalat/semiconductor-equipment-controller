/**
 * @file semaphore_producer_consumer.c
 *
 * @brief Producer-consumer queue using semaphores, mutex and graceful shutdown.
 *
 * This program demonstrates a bounded producer-consumer architecture using:
 *
 *     - POSIX threads
 *     - Counting semaphores
 *     - Mutex
 *     - Circular queue
 *     - Graceful worker shutdown
 *
 * Synchronization objects:
 *
 *     empty_slots
 *         Number of free positions in the queue.
 *
 *     available_tasks
 *         Number of tasks currently available in the queue.
 *
 *     mutex
 *         Protects the queue data.
 *
 * Shutdown behavior:
 *
 *     1. Producer creates all tasks.
 *     2. Producer finishes.
 *     3. Main sets shutdown = 1.
 *     4. Main wakes waiting workers using sem_post().
 *     5. Workers process remaining tasks.
 *     6. When no tasks remain and shutdown is set, workers exit.
 */

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>

#define QUEUE_SIZE 5
#define WORKER_COUNT 2
#define TASK_COUNT 8


/**
 * @brief Represents one task in the queue.
 */
typedef struct
{
    int id;
    char command[32];

} Task;


/**
 * @brief Bounded producer-consumer queue.
 */
typedef struct
{
    Task tasks[QUEUE_SIZE];

    /*
     * Index of the next task to remove.
     */
    int head;

    /*
     * Index where the next task will be inserted.
     */
    int tail;

    /*
     * Number of tasks currently stored in the queue.
     */
    int count;

    /*
     * Indicates that the producer has finished and
     * no new tasks will be added.
     */
    int shutdown;

    /*
     * Protects head, tail, count, tasks and shutdown.
     */
    pthread_mutex_t mutex;

    /*
     * Number of free positions in the queue.
     */
    sem_t empty_slots;

    /*
     * Number of tasks currently available.
     */
    sem_t available_tasks;

} TaskQueue;


/*
 * Global queue shared by producer and workers.
 */
TaskQueue queue;


/**
 * @brief Adds a task to the queue.
 *
 * The caller must first acquire an empty slot using
 * sem_wait(&queue.empty_slots).
 *
 * @param task Task to add.
 */
void enqueue_task(Task task)
{
    pthread_mutex_lock(&queue.mutex);


    /*
     * Store the task at the current tail position.
     */
    queue.tasks[queue.tail] = task;


    /*
     * Move tail to the next circular position.
     */
    queue.tail =
        (queue.tail + 1) % QUEUE_SIZE;


    /*
     * One more task is now present in the queue.
     */
    queue.count++;


    printf( "Producer: Added task %d (%s)\n", task.id, task.command);


    pthread_mutex_unlock(&queue.mutex);


    /*
     * Tell a waiting worker that a task is available.
     */
    sem_post(&queue.available_tasks);
}


/**
 * @brief Removes one task from the queue.
 *
 * The caller must have acquired an available task semaphore
 * before calling this function.
 *
 * @return Task removed from the queue.
 */
Task dequeue_task(void)
{
    Task task;


    pthread_mutex_lock(&queue.mutex);


    /*
     * Read the task at the current head position.
     */
    task = queue.tasks[queue.head];


    /*
     * Move head to the next circular position.
     */
    queue.head = (queue.head + 1) % QUEUE_SIZE;


    /*
     * One task has been removed.
     */
    queue.count--;


    pthread_mutex_unlock(&queue.mutex);


    /*
     * One queue position is now free.
     */
    sem_post(&queue.empty_slots);


    return task;
}


/**
 * @brief Producer thread.
 *
 * Creates TASK_COUNT tasks and inserts them into the queue.
 *
 * @param arg Unused.
 *
 * @return NULL.
 */
void *producer(void *arg)
{
    (void)arg;

    const char *commands[] =
    {
        "START",
        "STATUS",
        "ALARM",
        "STOP",
        "RESET",
        "STATUS",
        "START",
        "STOP"
    };


    for (int i = 0; i < TASK_COUNT; i++)
    {
        Task task;

        task.id = i + 1;

        snprintf( task.command, sizeof(task.command), "%s", commands[i]);


        /*
         * Wait for an empty queue slot.
         *
         * If the queue is full, producer blocks here.
         */
        sem_wait(&queue.empty_slots);


        /*
         * Add task to queue.
         */
        enqueue_task(task);

        sleep(1);
    }

    printf( "Producer: Finished producing tasks.\n" );

    return NULL;
}


/**
 * @brief Worker thread.
 *
 * Workers continue processing tasks until:
 *
 *     1. Producer has finished.
 *     2. No tasks remain in the queue.
 *
 * @param arg Pointer to worker ID.
 *
 * @return NULL.
 */
void *worker(void *arg)
{
    int worker_id = *(int *)arg;

    while (1)
    {
        /*
         * Wait for an available task.
         *
         * Normally this blocks when the queue is empty.
         */
        sem_wait(&queue.available_tasks);


        /*
         * We need to inspect both:
         *
         *     count
         *     shutdown
         *
         * while holding the mutex.
         */
        pthread_mutex_lock(&queue.mutex);


        /*
         * If no task remains AND the producer has finished,
         * there is nothing more for this worker to do.
         */
        if (queue.count == 0 && queue.shutdown)
        {
            pthread_mutex_unlock(&queue.mutex);

            break;
        }


        /*
         * A real task is available.
         *
         * Remove it while protected by the mutex.
         */
        Task task = queue.tasks[queue.head];

        queue.head = (queue.head + 1) % QUEUE_SIZE;

        queue.count--;

        pthread_mutex_unlock(&queue.mutex);


        /*
         * The queue now has one additional free position.
         */
        sem_post(&queue.empty_slots);


        /*
         * Process the task outside the mutex.
         *
         * This is important:
         *
         * We should not hold the queue mutex while performing
         * potentially slow work.
         */
        printf( "Worker %d: Processing task %d (%s)\n", worker_id, task.id, task.command);

        sleep(2);
    }

    printf( "Worker %d: Shutting down.\n", worker_id );

    return NULL;
}


/**
 * @brief Program entry point.
 *
 * Initializes the queue, creates workers and producer,
 * waits for completion, performs graceful shutdown and
 * finally destroys synchronization objects.
 *
 * @return 0 on success.
 */
int main(void)
{
    pthread_t producer_thread;

    pthread_t worker_threads[WORKER_COUNT];

    int worker_ids[WORKER_COUNT];


    /*
     * Initialize queue state.
     */
    queue.head = 0;
    queue.tail = 0;
    queue.count = 0;
    queue.shutdown = 0;


    /*
     * Initialize queue mutex.
     */
    pthread_mutex_init( &queue.mutex, NULL );


    /*
     * Initially the queue is completely empty.
     *
     * Therefore all queue positions are available.
     */
    sem_init( &queue.empty_slots, 0, QUEUE_SIZE );


    /*
     * Initially there are no tasks available.
     */
    sem_init( &queue.available_tasks, 0, 0);


    /*
     * Create worker threads.
     */
    for (int i = 0; i < WORKER_COUNT; i++)
    {
        worker_ids[i] = i + 1;

        pthread_create( &worker_threads[i], NULL, worker, &worker_ids[i] );
    }


    /*
     * Create producer.
     */
    pthread_create( &producer_thread, NULL, producer, NULL );


    /*
     * Wait until producer has created all tasks.
     */
    pthread_join( producer_thread, NULL );


    /*
     * Producer has finished.
     *
     * No new tasks will be added from now on.
     */
    pthread_mutex_lock(&queue.mutex);

    queue.shutdown = 1;

    pthread_mutex_unlock(&queue.mutex);


    printf( 
        "Main: Producer finished. "
        "Starting worker shutdown.\n"
    );


    /*
     * Wake all workers.
     *
     * Some workers may currently be blocked inside
     * sem_wait(&queue.available_tasks).
     *
     * Posting once for each worker gives every worker
     * an opportunity to wake up and check the shutdown state.
     */
    for (int i = 0; i < WORKER_COUNT; i++)
    {
        sem_post(&queue.available_tasks);
    }


    /*
     * Wait for all workers to finish.
     */
    for (int i = 0; i < WORKER_COUNT; i++)
    {
        pthread_join( worker_threads[i], NULL );
    }


    /*
     * All threads have completed.
     *
     * Synchronization objects can now safely be destroyed.
     */
    sem_destroy(&queue.empty_slots);

    sem_destroy(&queue.available_tasks);

    pthread_mutex_destroy(&queue.mutex);


    printf( "Program completed.\n" );


    return 0;
}