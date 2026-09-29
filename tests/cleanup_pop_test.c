/**
 * @file cleanup_pop_test.c
 *
 * @brief Demonstrates pthread_cleanup_push() and
 *        pthread_cleanup_pop() behavior.
 *
 * This program demonstrates the difference between:
 *
 *     pthread_cleanup_pop(0)
 *     pthread_cleanup_pop(1)
 *
 * pop(0):
 *     Removes the cleanup handler without executing it.
 *
 * pop(1):
 *     Executes the cleanup handler and then removes it.
 */

#include <stdio.h>
#include <pthread.h>


/**
 * @brief Cleanup handler.
 *
 * This function is called either:
 *
 * 1. When the thread is cancelled, or
 * 2. When pthread_cleanup_pop(1) is executed.
 */
void cleanup(void *arg)
{
    (void)arg;

    printf("Cleanup: Cleanup function executed.\n");
}


/**
 * @brief Worker thread.
 *
 * Demonstrates pop(0) and pop(1).
 */
void *worker(void *arg)
{
    (void)arg;

    printf("Worker: Started.\n");


    /*
     * Register cleanup handler.
     */
    pthread_cleanup_push( cleanup, NULL );

    printf("Worker: Doing some work...\n");


    /*
     * Remove cleanup handler.
     *
     * Argument 0 means:
     *
     *     Do NOT execute cleanup now.
     */
    pthread_cleanup_pop(0);

    printf("Worker: First cleanup handler removed.\n");

    /*
     * Register another cleanup handler.
     */
    pthread_cleanup_push( cleanup, NULL );

    printf("Worker: Doing more work...\n");

    /*
     * Argument 1 means:
     *
     *     Execute cleanup now.
     *
     * After execution, the handler is removed.
     */
    pthread_cleanup_pop(1);

    printf("Worker: Finished normally.\n");

    return NULL;
}


int main(void)
{
    pthread_t thread;

    printf("Main: Creating worker.\n");

    pthread_create(  &thread, NULL, worker, NULL );

    pthread_join( thread, NULL );

    printf("Main: Worker joined.\n");

    return 0;
}