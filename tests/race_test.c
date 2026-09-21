#include <stdio.h>
#include <pthread.h>

int counter = 0;

pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

void *worker(void *arg)
{
    for (int i = 0; i < 1000000; i++)
    {
        pthread_mutex_lock(&counter_mutex);

        // Critical section
        counter++;
        
        pthread_mutex_unlock(&counter_mutex);
    }

    return NULL;
}

int main(void)
{
    pthread_t thread1;
    pthread_t thread2;

    pthread_create(
        &thread1,
        NULL,
        worker,
        NULL);

    pthread_create(
        &thread2,
        NULL,
        worker,
        NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("Final counter = %d\n", counter);

    return 0;
}