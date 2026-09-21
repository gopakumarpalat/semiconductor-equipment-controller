#include <stdio.h>
#include <pthread.h>
#include "equipment.h"

void *equipment_worker(void *arg)
{
    Equipment *equipment = (Equipment*) arg;

    printf("Equipment thread is running.\n");
    printf("Equipment ID   : %d\n", equipment->id);
    printf("Equipment Name : %s\n", equipment->name);

}

int main()
{
    pthread_t thread;
    Equipment equipment;
    equipment_create( &equipment, 101, "ETCH01");

    printf("Main thread created equipment.\n");

     pthread_create(
        &thread,
        NULL,
        equipment_worker,
        &equipment);

    pthread_join(thread, NULL);

    printf("Main thread finished.\n");

    return 0;
}