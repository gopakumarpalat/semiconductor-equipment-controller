#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int data_ready = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;


void *worker(void *arg)
{
    (void)arg;

    printf("Worker: Waiting for data...\n");

    pthread_mutex_lock(&mutex);

    while (!data_ready)
    {
        pthread_cond_wait( &condition, &mutex );
    }

    printf("Worker: Data is ready!\n");

    pthread_mutex_unlock(&mutex);

    return NULL;
}


void *producer(void *arg)
{
    (void)arg;

    printf("Producer: Preparing data...\n");

    sleep(2);

    pthread_mutex_lock(&mutex);

    data_ready = 1;

    printf("Producer: Data is ready. Signaling worker...\n");

    pthread_cond_signal(&condition);

    pthread_mutex_unlock(&mutex);

    return NULL;
}


int main(void)
{
    pthread_t worker_thread;
    pthread_t producer_thread;

    pthread_create( &worker_thread, NULL, worker, NULL );

    pthread_create( &producer_thread, NULL, producer, NULL );

    pthread_join( worker_thread, NULL );

    pthread_join( producer_thread, NULL );

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condition);

    printf("Program completed.\n");

    return 0;
}