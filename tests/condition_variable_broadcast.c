#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int data_ready = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;


void *worker(void *arg)
{
    (void)arg;

    printf( "Worker %lu: Waiting for data...\n", (unsigned long)pthread_self());

    pthread_mutex_lock(&mutex);

    while (!data_ready)
    {
        pthread_cond_wait( &condition, &mutex );
    }

    printf( "Worker %lu: Data is ready!\n", (unsigned long)pthread_self() );

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

    //pthread_cond_signal(&condition);
    pthread_cond_broadcast(&condition);

    pthread_mutex_unlock(&mutex);

    return NULL;
}


int main(void)
{
    pthread_t worker1;
    pthread_t worker2;
    pthread_t worker3;
    pthread_t producer_thread;

    pthread_create(&worker1, NULL, worker, NULL);
    pthread_create(&worker2, NULL, worker, NULL);
    pthread_create(&worker3, NULL, worker, NULL);
    pthread_create(&producer_thread, NULL, producer, NULL);

    pthread_join(worker1, NULL);
    pthread_join(worker2, NULL);
    pthread_join(worker3, NULL);
    pthread_join(producer_thread, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condition);

    printf("Program completed.\n");

    return 0;
}