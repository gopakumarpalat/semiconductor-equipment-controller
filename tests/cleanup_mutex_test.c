/**
 * @file cleanup_mutex_test.c
 *
 * @brief Demonstrates pthread cleanup handlers.
 *
 * This program shows why cleanup handlers are useful
 * when a thread is cancelled while holding a mutex.
 */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>


pthread_mutex_t mutex;


/**
 * @brief Cleanup function for the mutex.
 *
 * This function is automatically executed when the
 * cleanup handler is triggered.
 */
void cleanup_mutex(void *arg)
{
    pthread_mutex_t *mutex = (pthread_mutex_t *)arg;

    printf("Cleanup: Unlocking mutex.\n");

    pthread_mutex_unlock(mutex);
}


/**
 * @brief Worker thread.
 */
void *worker(void *arg)
{
    (void)arg;

    printf("Worker: Locking mutex.\n");

    pthread_mutex_lock(&mutex);

    printf("Worker: Mutex locked.\n");


    /*
     * Register cleanup handler.
     *
     * If this thread is cancelled while the cleanup
     * handler is active, cleanup_mutex() will be called.
     */
    pthread_cleanup_push( cleanup_mutex, &mutex );


    printf("Worker: Doing work...\n");

    /*
     * Simulate long-running work.
     *
     * sleep() is a cancellation point.
     */
    sleep(10);


    /*
     * Normal execution reaches here.
     *
     * pop(1) means:
     *
     *     1. Remove cleanup handler.
     *     2. Execute cleanup handler now.
     *
     * Therefore cleanup_mutex() unlocks the mutex.
     */
    pthread_cleanup_pop(1);


    printf("Worker: Finished normally.\n");

    return NULL;
}


int main(void)
{
    pthread_t worker_thread;


    /*
     * Initialize mutex.
     */
    pthread_mutex_init(&mutex, NULL);


    /*
     * Create worker.
     */
    pthread_create( &worker_thread, NULL, worker, NULL );


    /*
     * Give the worker enough time to:
     *
     *     1. Start
     *     2. Lock the mutex
     *     3. Enter sleep()
     */
    sleep(2);


    printf("Main: Cancelling worker.\n");


    /*
     * Request cancellation of the worker.
     */
    pthread_cancel(worker_thread);


    /*
     * Wait for the worker to terminate.
     */
    pthread_join( worker_thread, NULL );


    printf("Main: Worker joined.\n");


    /*
     * Destroy mutex after the worker has finished.
     */
    pthread_mutex_destroy(&mutex);


    return 0;
}