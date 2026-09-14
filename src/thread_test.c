#include <stdio.h>
#include <pthread.h>

void *worker(void *arg)
{
    printf("Worker thread is running.\n");
    int *value = (int*)arg;
    printf("Worker received value = %d\n", *value);

    return NULL;
}

int main(void)
{
    pthread_t thread;
    int number = 100;

    printf("Main thread is running.\n");

    pthread_create(
        &thread,
        NULL,
        worker,
        &number);

    pthread_join(thread, NULL);

    printf("Main thread finished.\n");

    return 0;
}