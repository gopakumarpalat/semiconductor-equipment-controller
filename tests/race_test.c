#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

int counter = 0;

pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

void *worker(void *arg)
{
    // (void)arg; // To avoid warning
    int* result = NULL;

    if (arg != NULL)
    {
        int value = *(int *)arg;

        printf("Value passed from pthread_create is : %d\n", value);

        result = malloc(sizeof(int));

        *result = value * 2;
    }

    

    for (int i = 0; i < 1000000; i++)
    {
        pthread_mutex_lock(&counter_mutex);

        // Critical section
        counter++;
        
        pthread_mutex_unlock(&counter_mutex);
    }

    return result;
}

int main(void)
{
    pthread_t thread1;
    pthread_t thread2;

    int value = 100;

    void *thread_result;

    pthread_create(
        &thread1, // thread
        NULL,
        worker/*thread function*/,
        &value/*Arguments*/);

    pthread_create(
        &thread2,
        NULL,
        worker,
        NULL);

    pthread_join(thread1, &thread_result/*return value from thread*/);
    pthread_join(thread2, NULL);

    int result = *(int *)thread_result;

    printf("Thread result = %d\n", result);

    free(thread_result); // Memory created in thread function using malloc and releasing that memory here!

    printf("Final counter = %d\n", counter);

    return 0;
}