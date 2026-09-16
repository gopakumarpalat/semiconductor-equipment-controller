#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#define SHM_NAME "/equipment_shm"

typedef struct
{
    int state;
    int temperature;
    int pressure;
    char equipment_name[32];

} EquipmentData;

int main(void)
{
    int fd;
    EquipmentData *data;

    /* Create shared memory */
    fd = shm_open(
        SHM_NAME,
        O_CREAT | O_RDWR,
        0666);

    if (fd == -1)
    {
        perror("shm_open");
        return 1;
    }

    /* Set shared memory size */
    if (ftruncate(fd, sizeof(EquipmentData)) == -1)
    {
        perror("ftruncate");
        return 1;
    }

    /* Map shared memory */
    data = mmap(
        NULL,
        sizeof(EquipmentData),
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd,
        0);

    if (data == MAP_FAILED)
    {
        perror("mmap");
        return 1;
    }

    close(fd);

    /* Write equipment data */
    data->state = 2;
    data->temperature = 75;
    data->pressure = 120;

    strcpy(data->equipment_name, "ETCH01");

    printf("Writer: shared data updated.\n");
    printf("Equipment : %s\n", data->equipment_name);
    printf("State      : %d\n", data->state);
    printf("Temperature: %d\n", data->temperature);
    printf("Pressure   : %d\n", data->pressure);

    printf("\nWriter: keeping shared memory alive...\n");

    sleep(10);

    /* Unmap */
    munmap(data, sizeof(EquipmentData));

    /* Remove shared memory */
    shm_unlink(SHM_NAME);

    printf("Writer: shared memory removed.\n");

    return 0;
}