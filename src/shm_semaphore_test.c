#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>

typedef struct
{
    int state;
    int temperature;
    int pressure;
    char equipment_name[32];

} EquipmentData;

typedef struct
{
    sem_t semaphore;
    EquipmentData data;

} SharedData;

int main(void)
{
    SharedData *shared;

    /* Create shared memory */
    shared = mmap( NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (shared == MAP_FAILED)
    {
        perror("mmap");
        return 1;
    }

    /* Initialize semaphore */
    sem_init( &shared->semaphore, 1, 1);

    /* Initialize equipment data */
    shared->data.state = 1;
    shared->data.temperature = 25;
    shared->data.pressure = 100;

    snprintf( shared->data.equipment_name, sizeof(shared->data.equipment_name), "ETCH01");


    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return 1;
    }


    if (pid == 0)
    {
        /* =========================
           Child = Equipment Controller
           ========================= */

        printf("Child: Equipment Controller started.\n");

        printf("Child: waiting for semaphore...\n");

        sem_wait(&shared->semaphore);

        printf("Child: updating equipment data...\n");

        shared->data.state = 2;
        shared->data.temperature = 75;
        shared->data.pressure = 120;

        printf("Child: data updated.\n");

        sem_post(&shared->semaphore);

        printf("Child: semaphore released.\n");

        return 0;
    }
    else
    {
        /* =========================
           Parent = Monitor
           ========================= */

        wait(NULL);

        printf("\nParent: reading equipment data...\n");

        sem_wait(&shared->semaphore);

        printf( "Equipment : %s\n", shared->data.equipment_name);

        printf( "State      : %d\n", shared->data.state);

        printf( "Temperature: %d\n", shared->data.temperature);

        printf( "Pressure   : %d\n", shared->data.pressure);

        sem_post(&shared->semaphore);

        printf("Parent: semaphore released.\n");

        sem_destroy(&shared->semaphore);

        munmap( shared, sizeof(SharedData));

        return 0;
    }
}