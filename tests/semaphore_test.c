/**
 * @file semaphore_test.c
 *
 * @brief Demonstrates POSIX counting semaphore usage with multiple threads.
 *
 * This program demonstrates how a counting semaphore can be used to limit
 * the number of threads that can access a limited resource at the same time.
 *
 * In this example:
 *
 *     - 5 worker threads are created.
 *     - Only 2 resources are available.
 *     - Therefore, at most 2 workers can hold a resource simultaneously.
 *
 * The semaphore is initialized with a value of 2.
 *
 *     sem_wait()
 *         -> Acquires one available resource.
 *         -> Decrements the semaphore count.
 *         -> Blocks if the semaphore count is already zero.
 *
 *     sem_post()
 *         -> Releases one resource.
 *         -> Increments the semaphore count.
 *         -> Allows a waiting thread to continue.
 *
 * This is a counting semaphore because the semaphore value can represent
 * multiple available resources.
 *
 * @note
 * sem_init() is used with pshared = 0 because the semaphore is shared
 * between threads belonging to the same process.
 */

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

/*
 * Number of worker threads created by the program.
 *
 * Five workers will compete for the available resources.
 */
#define THREAD_COUNT 5

/*
 * Number of resources available.
 *
 * The semaphore is initialized with this value.
 *
 * Since RESOURCE_COUNT is 2, at most two workers can acquire a resource
 * at the same time.
 */
#define RESOURCE_COUNT 2


/*
 * Global counting semaphore.
 *
 * The semaphore keeps track of how many resources are currently available.
 *
 * Initial value:
 *
 *     2
 *
 * Example:
 *
 *     Worker 1 -> sem_wait() -> 1
 *     Worker 2 -> sem_wait() -> 0
 *     Worker 3 -> sem_wait() -> WAIT
 *
 * When a worker releases a resource:
 *
 *     sem_post() -> semaphore count increases
 *
 * A waiting worker can then continue.
 */
sem_t resource_semaphore;



/**
 * @brief Worker thread function.
 *
 * Each worker tries to acquire one resource using sem_wait().
 * If no resource is available, the worker blocks until another worker
 * releases a resource using sem_post().
 *
 * @param arg Pointer to the worker ID.
 *
 * @return NULL when the worker finishes.
 */
void *worker(void *arg)
{
    /*
     * pthread_create() passes the worker ID as a void pointer.
     *
     * Convert the generic pointer back to int pointer and dereference it
     * to obtain the actual worker ID.
     */
    int worker_id = *(int *)arg;

    /*
     * The worker is ready to request a resource.
     */
    printf("Worker %d: Waiting for resource...\n", worker_id);

    /*
     * Try to acquire one resource.
     *
     * sem_wait() performs two possible actions:
     *
     * 1. If semaphore value > 0:
     *        Decrement the value and continue.
     *
     * 2. If semaphore value == 0:
     *        Block the current thread until a resource becomes available.
     */
    sem_wait(&resource_semaphore);

    /*
     * Execution reaches here only after the worker has successfully
     * acquired a resource.
     */
    printf("Worker %d: Acquired resource.\n", worker_id);

    /*
     * Simulate using the resource.
     *
     * In a real equipment-control application, this could represent
     * operations such as:
     *
     *     - communicating with hardware
     *     - accessing a limited device
     *     - performing a machine operation
     *     - using a limited connection/channel
     *
     * sleep(2) simply keeps the resource occupied for two seconds so
     * that the semaphore behavior can be observed easily.
     */
    sleep(2);

    /*
     * The worker has finished using the resource.
     *
     * Release the resource back to the semaphore.
     *
     * sem_post() increments the semaphore value and can wake a thread
     * that is currently blocked inside sem_wait().
     */
    printf("Worker %d: Releasing resource.\n", worker_id);

    sem_post(&resource_semaphore);

    /*
     * Worker thread has completed its work.
     */
    return NULL;
}



/**
 * @brief Program entry point.
 *
 * Creates multiple worker threads and initializes a counting semaphore
 * that limits concurrent resource usage.
 *
 * @return 0 on successful completion.
 */
int main(void)
{
    /*
     * Array containing the thread handles returned by pthread_create().
     */
    pthread_t threads[THREAD_COUNT];


    /*
     * Each thread receives a unique worker ID.
     *
     * Example:
     *
     *     worker_ids[0] = 1
     *     worker_ids[1] = 2
     *     ...
     *     worker_ids[4] = 5
     *
     * The address of each element is passed to the corresponding thread.
     */
    int worker_ids[THREAD_COUNT];


    /*
     * Initialize the counting semaphore.
     *
     * Parameters:
     *
     *     &resource_semaphore
     *         -> Semaphore object to initialize.
     *
     *     0
     *         -> pshared parameter.
     *            0 means the semaphore is shared between threads
     *            within the same process.
     *
     *     RESOURCE_COUNT
     *         -> Initial semaphore value.
     *            Here it is 2, meaning two resources are available.
     *
     * Therefore:
     *
     *     Initial semaphore count = 2
     */
    sem_init( &resource_semaphore, 0, RESOURCE_COUNT);

    /*
     * Create all worker threads.
     */
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        /*
         * Worker IDs start from 1 instead of 0 to make the output
         * easier to read.
         */
        worker_ids[i] = i + 1;

        /*
         * Create a worker thread.
         *
         * The address of worker_ids[i] is passed as the thread argument.
         */
        pthread_create(&threads[i], NULL, worker, &worker_ids[i]);
    }

    /*
     * Wait for every worker thread to finish.
     *
     * pthread_join() prevents main() from terminating before the
     * worker threads complete their work.
     */
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        pthread_join(threads[i], NULL);
    }


    /*
     * All workers have completed.
     *
     * The semaphore is no longer required, so release its resources.
     */
    sem_destroy(&resource_semaphore);

    /*
     * Indicate that the complete program has finished successfully.
     */
    printf("Program completed.\n");

    return 0;
}