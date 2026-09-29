/**
 * @file binary_semaphore_test.c
 *
 * @brief Demonstrates the use of a binary semaphore with multiple threads.
 *
 * A binary semaphore normally has a value of either:
 *
 *     1 -> resource available
 *     0 -> resource unavailable
 *
 * This example creates multiple worker threads that compete for one
 * shared resource.
 *
 * Since the semaphore is initialized with value 1, only one worker
 * can access the resource at a time.
 */

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>


/*
 * Number of worker threads.
 */
#define THREAD_COUNT 4


/*
 * Binary semaphore.
 *
 * Initial value will be 1.
 *
 * Therefore, one worker can acquire the resource at a time.
 */
sem_t resource_semaphore;


/**
 * @brief Worker thread function.
 *
 * Each worker waits for the binary semaphore before accessing
 * the simulated shared resource.
 *
 * @param arg Pointer to the worker ID.
 *
 * @return NULL when the worker finishes.
 */
void *worker(void *arg)
{
    /*
     * Convert the generic thread argument back to an integer pointer.
     */
    int worker_id = *(int *)arg;

    printf("Worker %d: Waiting for resource...\n", worker_id);


    /*
     * Acquire the resource.
     *
     * Because this is a binary semaphore:
     *
     *     1 -> 0
     *
     * If the value is already 0, this thread blocks.
     */
    sem_wait(&resource_semaphore);

    printf("Worker %d: Acquired resource.\n", worker_id);

    /*
     * Simulate using the shared resource.
     */
    sleep(2);

    printf("Worker %d: Releasing resource.\n", worker_id);


    /*
     * Release the resource.
     *
     *     0 -> 1
     *
     * A waiting worker can now acquire the resource.
     */
    sem_post(&resource_semaphore);
    
    return NULL;
}


/**
 * @brief Program entry point.
 *
 * @return 0 on successful completion.
 */
int main(void)
{
    pthread_t threads[THREAD_COUNT];

    int worker_ids[THREAD_COUNT];

    /*
     * Initialize the binary semaphore.
     *
     * pshared = 0
     *     -> semaphore is shared between threads in this process.
     *
     * initial value = 1
     *     -> resource is initially available.
     */
    sem_init( &resource_semaphore, 0, 1);

    /*
     * Create worker threads.
     */
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        worker_ids[i] = i + 1;

        pthread_create( &threads[i], NULL, worker, &worker_ids[i]);
    }

    /*
     * Wait for all workers to finish.
     */
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        pthread_join(threads[i], NULL);
    }

    /*
     * Destroy the semaphore after all threads have finished.
     */
    sem_destroy(&resource_semaphore);

    printf("Program completed.\n");

    return 0;
}