/**
 * @file equipment_worker_system.c
 *
 * @brief Day 11 - Advanced Multithreading
 *        Final Mini Project
 *
 * This program demonstrates a producer-consumer
 * equipment command processing system using:
 *
 *     - Circular command queue
 *     - POSIX mutex
 *     - POSIX semaphore
 *     - Producer thread
 *     - Multiple worker threads
 *     - Worker-specific context
 *     - Graceful shutdown
 *     - pthread cleanup handlers
 *
 * Architecture:
 *
 *     Producer thread
 *             |
 *             v
 *       Circular Queue
 *             |
 *        Semaphore
 *             |
 *       +-----+-----+
 *       |           |
 *       v           v
 *   Worker 1     Worker 2
 *
 * The queue mutex protects:
 *
 *     - head
 *     - tail
 *     - count
 *     - shutdown
 *     - command data
 */


#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

/*
 * Maximum number of commands that can be stored
 * in the queue at one time.
 */
#define QUEUE_SIZE 8
#define WORKER_COUNT 2

sem_t queue_sem;


/**
 * @brief Type of equipment command.
 */
typedef enum
{
    CMD_START,
    CMD_STOP,
    CMD_STATUS,
    CMD_SHUTDOWN

} CommandType;


/**
 * @brief Represents one equipment command.
 *
 * Example:
 *
 *     CMD_START + "ETCH01"
 *
 * means:
 *
 *     START ETCH01
 */
typedef struct
{
    CommandType type;

    char equipment_name[32];

} Command;






/**
 * @brief Circular command queue.
 *
 * head:
 *     Points to the next command that should be removed.
 *
 * tail:
 *     Points to the next empty position where a new
 *     command will be inserted.
 *
 * count:
 *     Number of commands currently stored.
 */
typedef struct
{
    Command commands[QUEUE_SIZE];

    int head;
    int tail;
    int count;

    int shutdown;

    pthread_mutex_t mutex;

} CommandQueue;



typedef struct
{
    CommandQueue *queue;
    int worker_id;

} WorkerContext;



/**
 * @brief Initialize the command queue.
 */
void queue_init(CommandQueue *queue)
{
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;

    queue->shutdown = 0;

    pthread_mutex_init(&queue->mutex, NULL);
}



/**
 * @brief Add a command to the queue.
 *
 * @return
 *     0  - success
 *    -1  - queue is full
 */
int queue_push( CommandQueue *queue, Command command )
{
    pthread_mutex_lock(&queue->mutex);

    /*
     * Check whether queue is full.
     */
    if (queue->count == QUEUE_SIZE)
    {
        pthread_mutex_unlock(&queue->mutex);
        return -1;
    }

    /*
     * Store command at tail position.
     */
    queue->commands[queue->tail] = command;

    /*
     * Move tail to the next position.
     *
     * % QUEUE_SIZE makes the queue circular.
     *
     * Example:
     *
     *     0 -> 1 -> 2 -> ... -> 7 -> 0
     */
    queue->tail = (queue->tail + 1) % QUEUE_SIZE;


    /*
     * One more command is now stored.
     */
    queue->count++;

    pthread_mutex_unlock(&queue->mutex);

    return 0;
}



/**
 * @brief Remove a command from the queue.
 *
 * @return
 *     0  - success
 *    -1  - queue is empty
 */
int queue_pop( CommandQueue *queue, Command *command )
{
    pthread_mutex_lock(&queue->mutex);

    /*
     * Check whether queue is empty.
     */
    if (queue->count == 0)
    {
        pthread_mutex_unlock(&queue->mutex);
        return -1;
    }

    /*
     * Copy the oldest command to the caller.
     */
    *command = queue->commands[queue->head];

    /*
     * Move head to the next position.
     */
    queue->head = (queue->head + 1) % QUEUE_SIZE;

    /*
     * One command has been removed.
     */
    queue->count--;

    pthread_mutex_unlock(&queue->mutex);

    return 0;
}


/**
 * @brief Convert command type to readable text.
 */
const char *command_to_string( CommandType type )
{
    switch (type)
    {
        case CMD_START:
            return "START";

        case CMD_STOP:
            return "STOP";

        case CMD_STATUS:
            return "STATUS";

        case CMD_SHUTDOWN:
            return "SHUTDOWN";

        default:
            return "UNKNOWN";
    }
}


/**
 * @brief Producer thread.
 *
 * Generates equipment commands and places them
 * into the shared command queue.
 */
void *producer(void *arg)
{
    CommandQueue *queue = (CommandQueue *)arg;


    Command commands[] =
    {
        { CMD_START,  "ETCH01" },
        { CMD_STATUS, "CVD01"  },
        { CMD_STOP,   "ETCH01" },
        { CMD_START,  "CVD01"  },
    };

    int command_count = sizeof(commands) / sizeof(commands[0]);

    for (int i = 0; i < command_count; i++)
    {
        if (queue_push(queue, commands[i]) == 0)
        {
            printf( "Producer: Added %s for %s\n", command_to_string(commands[i].type), commands[i].equipment_name );

            sem_post(&queue_sem);
        }
        else
        {
            printf( "Producer: Queue is full!\n" );
        }
    }

    printf("Producer: Finished.\n");

    return NULL;
}


/**
 * @brief Cleanup handler for the queue mutex.
 *
 * This function is called if a worker thread is cancelled
 * while it is holding the queue mutex.
 *
 * @param arg Pointer to the queue mutex.
 */
void cleanup_queue_mutex(void *arg)
{
    pthread_mutex_t *mutex = (pthread_mutex_t *)arg;

    printf("Cleanup: Worker unlocking queue mutex.\n");

    pthread_mutex_unlock(mutex);
}


/**
 * @brief Worker thread.
 *
 * Removes commands from the shared queue and processes them.
 *
 * Synchronization:
 *
 *     sem_wait()
 *         |
 *         v
 *     Lock queue mutex
 *         |
 *         v
 *     Register cleanup handler
 *         |
 *         v
 *     Check shutdown / remove command
 *         |
 *         v
 *     cleanup_pop(1)
 *         |
 *         v
 *     Mutex unlocked
 *         |
 *         v
 *     Process command
 *
 * The cleanup handler protects the queue mutex in case
 * the worker is cancelled while the mutex is locked.
 */
void *worker(void *arg)
{
    WorkerContext *context = (WorkerContext*)arg;

    CommandQueue *queue = context->queue;

    int worker_id = context->worker_id;

    Command command;

    int should_exit;

    /*
     * Try to continuously get commands
     * from the queue.
     */
    while (1)
    {
        /*
         * Reset this flag for every loop iteration.
         */
        should_exit = 0;


        /*
         * Wait until:
         *
         *     - a real command becomes available
         *
         * OR
         *
         *     - main wakes us during shutdown.
         */
        sem_wait(&queue_sem);

        /*
         * Lock the queue mutex.
         *
         * From this point onward, the queue state
         * must be protected.
         */
        pthread_mutex_lock(&queue->mutex);

        /*
         * Register cleanup handler.
         *
         * If this worker is cancelled while the mutex
         * is locked, cleanup_queue_mutex() will execute
         * and unlock the mutex.
         */
        pthread_cleanup_push( cleanup_queue_mutex, &queue->mutex );

        /*
         * If:
         *
         *     1. No commands remain
         *     2. Shutdown has been requested
         *
         * then this worker can safely exit.
         */
        if (queue->count == 0 && queue->shutdown)
        {
            /*
             * Tell the code after cleanup_pop()
             * that this worker should exit.
             */
            should_exit = 1;
        }
        else
        {
            /*
             * A command is available.
             *
             * Remove the oldest command while the
             * queue mutex is locked.
             */
            command =
                queue->commands[queue->head];


            /*
             * Move head to the next circular position.
             */
            queue->head =
                (queue->head + 1) % QUEUE_SIZE;


            /*
             * One command has been removed.
             */
            queue->count--;
        }


        /*
         * Execute the cleanup handler now.
         *
         * cleanup_queue_mutex() unlocks the mutex.
         *
         * This is safe for normal execution.
         *
         * It also removes the cleanup handler so that
         * it is no longer registered after this point.
         */
        pthread_cleanup_pop(1);

        /*
         * If shutdown was requested and no commands
         * remained, leave the worker loop.
         */
        if (should_exit)
        {
            break;
        }


        /*
         * IMPORTANT:
         *
         * The mutex is already unlocked here.
         *
         * Therefore command processing does not block
         * other workers from accessing the queue.
         */
        printf( "Worker %d: Processing %s for %s\n", worker_id, command_to_string(command.type), command.equipment_name );
        
        
    }

    printf("Worker %d: Finished.\n", worker_id);

    return NULL;
}




/**
 * @brief Main function.
 *
 * Creates:
 *
 *     - One shared command queue
 *     - One producer thread
 *     - Multiple worker threads
 *
 * Shutdown sequence:
 *
 *     1. Producer creates all real commands.
 *     2. Main waits for producer to finish.
 *     3. Main sets queue.shutdown = 1.
 *     4. Main wakes all workers using sem_post().
 *     5. Workers check:
 *
 *            queue->count == 0
 *            &&
 *            queue->shutdown == 1
 *
 *        If both are true, the worker exits.
 */
int main(void)
{
    /*
     * Shared command queue.
     *
     * All threads use this same queue.
     */
    CommandQueue queue;

    /*
     * One context for each worker.
     *
     * Each context contains:
     *
     *     - pointer to the shared queue
     *     - unique worker ID
     */
    WorkerContext contexts[WORKER_COUNT];

    /*
     * Thread handles.
     */
    pthread_t producer_thread;
    pthread_t worker_threads[WORKER_COUNT];

    
    /*
     * Initialize the command queue.
     *
     * This initializes:
     *
     *     head     = 0
     *     tail     = 0
     *     count    = 0
     *     shutdown = 0
     *
     * and initializes the queue mutex.
     */
    queue_init(&queue);
    


    /*
     * Initialize semaphore.
     *
     * Initial value = 0 because the queue is
     * initially empty.
     *
     * Producer will call sem_post() whenever
     * it adds a command.
     */
    if (sem_init(&queue_sem, 0, 0) != 0)
    {
        printf("Failed to initialize semaphore.\n");

        pthread_mutex_destroy(&queue.mutex);

        return 1;
    }


    /*
     * Create the Producer thread.
     *
     * The Producer will generate commands and
     * add them to the shared command queue.
     *
     * &queue is passed as the thread argument.
     */
    pthread_create( &producer_thread, NULL, producer, &queue);


    /*
     * Create Worker threads.
     *
     * Every worker shares the same queue,
     * but receives a different WorkerContext.
     */
    for (int i = 0; i < WORKER_COUNT; i++)
    {
        /*
         * Each context points to the same queue,
         * because all workers share the same queue.
         */
        contexts[i].queue = &queue;

        /*
         * Give this worker a unique ID.
         *
         * Worker 1 -> ID 1
         * Worker 2 -> ID 2
         */
        contexts[i].worker_id = i + 1;

        /*
         * Pass this worker's own context.
         *
         * Worker 1 -> &contexts[0]
         * Worker 2 -> &contexts[1]
         */
        pthread_create( &worker_threads[i], NULL, worker, &contexts[i] );
    }


    /*
     * Wait for the Producer to finish.
     *
     * Producer creates all real commands and
     * then returns.
     */
    pthread_join( producer_thread, NULL );


    /*
     * Producer has finished.
     *
     * Therefore no more commands will be added
     * to the queue.
     *
     * Set the shared shutdown flag.
     *
     * The mutex protects the shutdown variable
     * because workers also access it.
     */
    pthread_mutex_lock(&queue.mutex);

    queue.shutdown = 1;

    pthread_mutex_unlock(&queue.mutex);


    /*
     * Wake all workers.
     *
     * Workers may currently be blocked inside:
     *
     *     sem_wait(&queue_sem);
     *
     * We post once for each worker so that every
     * worker gets an opportunity to wake up and
     * check the shutdown state.
     */
    for (int i = 0; i < WORKER_COUNT; i++)
    {
        sem_post(&queue_sem);
    }


    /*
     * Wait for all workers to finish.
     *
     * Worker shutdown will be handled inside
     * worker().
     */
    for (int i = 0; i < WORKER_COUNT; i++)
    {
        pthread_join(
            worker_threads[i],
            NULL
        );
    }

    /*
     * All threads have finished.
     *
     * It is now safe to destroy the synchronization
     * objects.
     */
    sem_destroy(&queue_sem);
    pthread_mutex_destroy(&queue.mutex);

    printf("Program completed.\n");

    return 0;
}