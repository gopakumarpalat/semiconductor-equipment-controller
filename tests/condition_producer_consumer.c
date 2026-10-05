#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int data_ready = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t condition = PTHREAD_COND_INITIALIZER;


/**
 * @brief Worker thread.
 *
 * Waits until the producer makes data available.
 */
void *worker(void *arg)
{
    (void)arg;

    printf("Worker: Started.\n");


    /*
     * Lock the mutex before checking
     * the shared condition.
     */
    pthread_mutex_lock(&mutex);


    /*
     * Wait while data is not ready.
     *
     * pthread_cond_wait() does two things:
     *
     *     1. Unlocks the mutex
     *     2. Puts this thread to sleep
     *
     * When the worker is signaled, it:
     *
     *     1. Wakes up
     *     2. Locks the mutex again
     *     3. Returns from pthread_cond_wait()
     */
    while (!data_ready)
    {
        printf("Worker: No data. Waiting...\n");

        pthread_cond_wait( &condition, &mutex );
    }


    /*
     * We are here because data_ready
     * is now true.
     */
    printf("Worker: Data is ready. Processing...\n");


    /*
     * Release the mutex.
     */
    pthread_mutex_unlock(&mutex);


    return NULL;
}


/**
 * @brief Producer thread.
 *
 * Produces data and notifies the worker.
 */
void *producer(void *arg)
{
    (void)arg;

    printf("Producer: Preparing data...\n");

    /*
     * Simulate some work.
     */
    sleep(2);


    /*
     * Lock the mutex before changing
     * the shared condition.
     */
    pthread_mutex_lock(&mutex);


    /*
     * Data is now available.
     */
    data_ready = 1;

    printf( "Producer: Data is ready.\n" );


    /*
     * Wake one thread waiting on
     * the condition variable.
     */
    pthread_cond_signal(&condition);


    /*
     * Release the mutex.
     */
    pthread_mutex_unlock(&mutex);

    return NULL;
}


int main(void)
{
    pthread_t worker_thread;
    pthread_t producer_thread;


    /*
     * Create worker first.
     */
    pthread_create( &worker_thread, NULL, worker, NULL );


    /*
     * Create producer.
     */
    pthread_create( &producer_thread, NULL, producer, NULL );


    /*
     * Wait for both threads.
     */
    pthread_join( worker_thread, NULL );

    pthread_join( producer_thread, NULL );


    /*
     * Destroy synchronization objects.
     */
    pthread_mutex_destroy(&mutex);

    pthread_cond_destroy(&condition);


    printf("Program completed.\n");

    return 0;
}