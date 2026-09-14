#include <stdio.h>
#include <pthread.h>
#include <windows.h>

pthread_mutex_t mutex_a = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_b = PTHREAD_MUTEX_INITIALIZER;

void *worker1(void *arg)
{
    printf("Thread 1: locking mutex A\n");

    pthread_mutex_lock(&mutex_a);

    printf("Thread 1: locked mutex A\n");

    Sleep(1000);

    printf("Thread 1: waiting for mutex B\n");

    pthread_mutex_lock(&mutex_b);

    printf("Thread 1: locked mutex B\n");

    pthread_mutex_unlock(&mutex_b);
    pthread_mutex_unlock(&mutex_a);

    return NULL;
}


// Deadlock origin
/*void *worker2(void *arg)
{
    printf("Thread 2: locking mutex B\n");

    pthread_mutex_lock(&mutex_b);

    printf("Thread 2: locked mutex B\n");

    Sleep(1000);

    printf("Thread 2: waiting for mutex A\n");

    pthread_mutex_lock(&mutex_a);

    printf("Thread 2: locked mutex A\n");

    pthread_mutex_unlock(&mutex_a);
    pthread_mutex_unlock(&mutex_b);

    return NULL;
}*/

//Deadlock resolving by correcting order of locking
void *worker2(void *arg)
{
    printf("Thread 2: locking mutex A\n");

    pthread_mutex_lock(&mutex_a);

    printf("Thread 2: locked mutex A\n");

    Sleep(1000);

    printf("Thread 2: waiting for mutex B\n");

    pthread_mutex_lock(&mutex_b);

    printf("Thread 2: locked mutex B\n");

    pthread_mutex_unlock(&mutex_b);
    pthread_mutex_unlock(&mutex_a);

    return NULL;
}

int main(void)
{
    pthread_t thread1;
    pthread_t thread2;

    pthread_create(
        &thread1,
        NULL,
        worker1,
        NULL);

    pthread_create(
        &thread2,
        NULL,
        worker2,
        NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("Program finished.\n");

    return 0;
}