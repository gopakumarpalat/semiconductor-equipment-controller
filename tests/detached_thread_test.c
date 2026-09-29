/**
 * @file detached_thread_test.c
 *
 * @brief Demonstrates a joinable POSIX thread.
 *
 * A thread created using pthread_create() is joinable by default.
 *
 * The main thread uses pthread_join() to wait for the worker
 * to finish.
 */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>


/**
 * @brief Worker thread function.
 *
 * @param arg Unused.
 * @return NULL.
 */
void *worker(void *arg)
{
    (void)arg;

    printf("Worker: Started.\n");

    for (int i = 1; i <= 5; i++)
    {
        printf("Worker: Working... %d\n", i);
        sleep(1);
    }

    printf("Worker: Finished.\n");

    return NULL;
}


int main(void)
{
    pthread_t worker_thread;

    printf("Main: Creating worker.\n");

    /*
     * Create a joinable thread.
     *
     * pthread_create() creates joinable threads by default.
     */
    pthread_create( &worker_thread, NULL, worker, NULL);

    pthread_detach(worker_thread);

    /**
     * If we add this, then only we can see worker thread execution.
     * Else when main process terminate, thread also terminate along with main.
     */
    sleep(7); 

    printf("Main finished.\n" );

    return 0;
}