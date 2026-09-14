#include <stdio.h>
#include <pthread.h>
#include <windows.h>

int command_available = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;

void *worker(void *arg)
{
    printf("Worker: started.\n");

    pthread_mutex_lock(&mutex);

    while (command_available == 0)
    {
        printf("Worker: no command. Waiting...\n");

        pthread_cond_wait(&condition, &mutex);
    }

    printf("Worker: command received!\n");

    command_available = 0;

    pthread_mutex_unlock(&mutex);

    return NULL;
}



int main(void)
{
    pthread_t worker_thread;

    pthread_create( &worker_thread, NULL, worker, NULL);

    Sleep(2000);

    printf("Main: sending command.\n");

    pthread_mutex_lock(&mutex);

    command_available = 1;

    pthread_cond_signal(&condition);

    pthread_mutex_unlock(&mutex);

    pthread_join(worker_thread, NULL);

    printf("Main: worker finished.\n");

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condition);

    return 0;
}