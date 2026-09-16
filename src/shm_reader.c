#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

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

    /* Open existing shared memory */
    fd = shm_open(
        SHM_NAME,
        O_RDONLY,
        0666);

    if (fd == -1)
    {
        perror("shm_open");
        return 1;
    }

    /* Map shared memory */
    data = mmap(
        NULL,
        sizeof(EquipmentData),
        PROT_READ,
        MAP_SHARED,
        fd,
        0);

    if (data == MAP_FAILED)
    {
        perror("mmap");
        return 1;
    }

    close(fd);

    printf("Reader: reading shared data.\n");

    printf("Equipment : %s\n", data->equipment_name);
    printf("State      : %d\n", data->state);
    printf("Temperature: %d\n", data->temperature);
    printf("Pressure   : %d\n", data->pressure);

    munmap(data, sizeof(EquipmentData));

    return 0;
}