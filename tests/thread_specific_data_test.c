/**
 * @file thread_specific_data_test.c
 *
 * @brief Demonstrates POSIX Thread-Specific Data (TSD).
 *
 * Each thread uses the SAME pthread_key_t,
 * but stores its OWN value using pthread_setspecific().
 *
 * pthread_getspecific() returns the value associated
 * with the CURRENT thread.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


/*
 * Global TSD key.
 *
 * The key itself is shared by all threads.
 * The value associated with the key is different
 * for each thread.
 */
pthread_key_t thread_key;


void destructor(void *value)
{
    printf(
        "Destructor: Cleaning TSD value = %d\n",
        *(int *)value
    );

    /*
     * Clean up memory allocated for this thread.
     */
    free(value);
}


/**
 * @brief Worker thread function.
 *
 * @param arg Pointer to an integer value.
 * @return NULL
 */
void *worker(void *arg)
{
    /*
     * Get the value passed through pthread_create().
     */
    int value = *(int *)arg;

    printf( "Thread: %lu - Input value = %d\n", (unsigned long)pthread_self(), value );


    /*
     * Store this value as Thread-Specific Data.
     *
     * This value belongs ONLY to the current thread.
     */
    int *thread_value = malloc(sizeof(int));

    if (thread_value == NULL)
    {
        printf("Memory allocation failed.\n");
        return NULL;
    }

    *thread_value = value;

    pthread_setspecific( thread_key, thread_value );


    /*
     * Retrieve the value from TSD.
     *
     * pthread_getspecific() returns the value
     * associated with the CURRENT thread.
     */
    int *stored_value =  pthread_getspecific(thread_key);


    if (stored_value != NULL)
    {
        printf( "Thread: %lu - TSD value = %d\n", (unsigned long)pthread_self(), *stored_value );
    }

    

    return NULL;
}


int main(void)
{
    pthread_t worker1;
    pthread_t worker2;
    pthread_t worker3;

    int value1 = 100;
    int value2 = 200;
    int value3 = 300;

    /*
     * Create the TSD key.
     *
     * NULL means we are not registering a destructor
     * function at this point.
     */
    if (pthread_key_create(&thread_key, destructor) != 0)
    {
        printf("Failed to create TSD key.\n");
        return 1;
    }

    /*
     * Create three threads.
     *
     * Each thread receives a different value through
     * pthread_create().
     */
    pthread_create( &worker1, NULL, worker, &value1 );

    pthread_create( &worker2, NULL, worker, &value2 );

    pthread_create( &worker3, NULL, worker, &value3 );


    /*
     * Wait for all threads.
     */
    pthread_join(worker1, NULL);
    pthread_join(worker2, NULL);
    pthread_join(worker3, NULL);

    /*
     * Delete the TSD key after all threads are finished.
     */
    pthread_key_delete(thread_key);

    printf("Main: Program completed.\n");

    return 0;
}