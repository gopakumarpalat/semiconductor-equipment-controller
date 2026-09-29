/**
 * @file thread_cancel_cleanup_test.c
 *
 * @brief Demonstrates cleanup handlers with POSIX thread cancellation.
 *
 * This example demonstrates why cleanup handlers are important when a
 * thread may be cancelled while holding a resource such as a mutex.
 *
 * Flow:
 *
 *     Worker
 *        |
 *        +--> Lock mutex
 *        |
 *        +--> Register cleanup handler
 *        |
 *        +--> Perform work
 *        |
 *        +--> Cancellation requested
 *        |
 *        +--> Cleanup handler runs
 *        |
 *        +--> Mutex is unlocked
 *        |
 *        +--> Worker terminates
 *
 * Without the cleanup handler, the mutex could remain locked after
 * thread cancellation.
 */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>


/*
 * Shared mutex.
 *
 * The worker will lock this mutex before performing its work.
 */
pthread_mutex_t resource_mutex =
    PTHREAD_MUTEX_INITIALIZER;


/**
 * @brief Cleanup function executed when the worker is cancelled.
 *
 * This function releases the mutex held by the worker.
 *
 * @param arg Pointer to the mutex that must be unlocked.
 */
void cleanup_mutex(void *arg)
{
    pthread_mutex_t *mutex =
        (pthread_mutex_t *)arg;


    printf(
        "Cleanup: Unlocking mutex...\n"
    );


    /*
     * Release the mutex so that other threads can use it.
     */
    pthread_mutex_unlock(mutex);
}


/**
 * @brief Worker thread function.
 *
 * The worker locks the mutex and then performs simulated work.
 *
 * A cleanup handler is registered immediately after the mutex is
 * acquired.
 *
 * If the worker is cancelled while performing the work, the cleanup
 * handler will unlock the mutex before the thread terminates.
 *
 * @param arg Unused.
 *
 * @return NULL if the worker exits normally.
 */
void *worker(void *arg)
{
    (void)arg;


    printf("Worker: Locking mutex...\n");


    /*
     * Acquire the shared resource.
     */
    pthread_mutex_lock(&resource_mutex);


    printf("Worker: Mutex locked.\n" );


    /*
     * Register cleanup handler.
     *
     * If this thread is cancelled, cleanup_mutex() will be called
     * automatically.
     */
    pthread_cleanup_push(cleanup_mutex, &resource_mutex );


    /*
     * Simulate work while holding the mutex.
     *
     * sleep() is a cancellation point.
     */
    while (1)
    {
        printf( "Worker: Doing protected work...\n" );

        sleep(1);
    }


    /*
     * pthread_cleanup_pop() must appear in the same lexical scope
     * as pthread_cleanup_push().
     *
     * The argument is 0 because normal execution never reaches this
     * point in this example.
     *
     * If cancellation happens, the cleanup handler is executed
     * automatically.
     */
    pthread_cleanup_pop(0);


    return NULL;
}


/**
 * @brief Program entry point.
 *
 * Creates the worker, allows it to run for a few seconds, requests
 * cancellation and waits for the worker to terminate.
 *
 * @return 0 on successful completion.
 */
int main(void)
{
    pthread_t worker_thread;


    /*
     * Create the worker thread.
     */
    pthread_create(
        &worker_thread,
        NULL,
        worker,
        NULL
    );


    /*
     * Allow the worker to acquire the mutex and perform work.
     */
    sleep(3);


    printf(
        "Main: Requesting worker cancellation...\n"
    );


    /*
     * Request cancellation.
     */
    pthread_cancel( worker_thread );


    /*
     * Wait until the worker has terminated.
     *
     * During cancellation, the cleanup handler should execute before
     * pthread_join() returns.
     */
    pthread_join( worker_thread, NULL );


    printf("Main: Worker terminated.\n");


    /*
     * Test whether the mutex was successfully released.
     *
     * If cleanup worked correctly, this lock should succeed.
     */
    printf("Main: Testing mutex after cancellation...\n");


    pthread_mutex_lock(&resource_mutex );


    printf("Main: Mutex successfully acquired.\n" );


    pthread_mutex_unlock( &resource_mutex );


    /*
     * Destroy the mutex because it is no longer needed.
     */
    pthread_mutex_destroy(&resource_mutex );


    printf("Program completed.\n" );


    return 0;
}