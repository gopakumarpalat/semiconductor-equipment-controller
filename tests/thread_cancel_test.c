/**
 * @file thread_cancel_test.c
 *
 * @brief Demonstrates POSIX thread cancellation.
 *
 * This program creates a worker thread that continuously performs
 * simulated work.
 *
 * The main thread waits for a few seconds and then sends a cancellation
 * request using pthread_cancel().
 *
 * Important:
 *
 *     pthread_cancel() sends a cancellation request.
 *
 * It does not simply mean "forcefully kill this thread immediately".
 *
 * The worker reaches a cancellation point at sleep(), where the
 * cancellation request can take effect.
 */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>


/**
 * @brief Worker thread function.
 *
 * The worker continuously performs simulated work.
 *
 * @param arg Unused.
 *
 * @return NULL if the worker exits normally.
 */
void *worker(void *arg)
{
    (void)arg;

    printf("Worker: Started.\n");

    while (1)
    {
        printf("Worker: Doing work...\n");


        /*
         * Simulate some work.
         *
         * sleep() is also a POSIX cancellation point.
         *
         * If another thread has requested cancellation,
         * the cancellation can take effect here.
         */
        sleep(1);
    }

    printf("This line won't execute!\n");


    return NULL;
}


/**
 * @brief Program entry point.
 *
 * @return 0 on successful completion.
 */
int main(void)
{
    pthread_t worker_thread;


    /*
     * Create the worker thread.
     */
    pthread_create( &worker_thread, NULL, worker, NULL);


    /*
     * Allow the worker to run for a few seconds.
     */
    sleep(3);


    printf( "Main: Requesting worker cancellation...\n");


    /*
     * Send a cancellation request to the worker.
     */
    pthread_cancel(worker_thread);


    /*
     * Wait for the worker to terminate.
     */
    pthread_join( worker_thread, NULL );


    printf( "Main: Worker terminated.\n");

    return 0;
}