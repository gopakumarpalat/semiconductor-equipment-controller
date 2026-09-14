#include <stdio.h>
#include <pthread.h>

void *robot_worker(void *arg)
{
    printf("Robot thread is running.\n");
}

void *sensor_worker(void *arg)
{
    printf("Sensor thread is running.\n");
}

void *network_worker(void *arg)
{
    printf("Network thread is running.\n");
}

int main()
{
    pthread_t robot_thread;
    pthread_t sensor_thread;
    pthread_t network_thread;

    printf("Main thread is running... \n");

    pthread_create(&robot_thread, NULL, robot_worker, NULL);
    pthread_create(&sensor_thread, NULL, sensor_worker, NULL);
    pthread_create(&network_thread, NULL, network_worker, NULL);

    pthread_join( robot_thread, NULL );
    pthread_join( sensor_thread, NULL );
    pthread_join( network_thread, NULL );

    printf("All worker threads finished.\n");
    printf("Main thread finished.\n");

    return 0;
}