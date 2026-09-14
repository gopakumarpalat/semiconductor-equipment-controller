#include <stdio.h>
#include <pthread.h>

#include "equipment.h"

void *equipment_worker(void *arg)
{
    WorkerData *data = (WorkerData *)arg;
    Equipment *equipment = data->equipment;

    for (int i = 0; i < 100000; i++)
    {
        pthread_mutex_lock(&equipment->mutex);

        equipment->counter++;

        pthread_mutex_unlock(&equipment->mutex);
    }

    return NULL;
}

int main(void)
{
    pthread_t thread1;
    pthread_t thread2;

    Equipment equipment;
    equipment_create( &equipment, 101, "ETCH01");

    WorkerData worker1;
    WorkerData worker2;

    worker1.equipment = &equipment;
    worker1.thread_id = 1;

    worker2.equipment = &equipment;
    worker2.thread_id = 2;    

    printf("Starting two threads for %s...\n\n", equipment.name);

    pthread_create( &thread1, NULL, equipment_worker, &worker1);

    pthread_create( &thread2, NULL, equipment_worker, &worker2);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("\nBoth threads finished.\n");

    printf( "Final counter = %d\n", equipment.counter);

    equipment_destroy(&equipment);

    return 0;
}