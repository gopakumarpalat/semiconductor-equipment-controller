#include <stdio.h>
#include "equipment.h"

enum EquipmentState
{
    IDLE,
    READY,
    RUNNING
};

int main()
{
    int command;

    printf("=================================\n");
    printf(" Semiconductor Equipment Control\n");
    printf("=================================\n");

    while (1)
    {
        printf("\n");

        equipment_print_state();

        printf("\n");
        printf("1. Initialize Equipment\n");
        printf("2. Start Equipment\n");
        printf("3. Stop Equipment\n");
        printf("4. Exit\n");

        printf("\nEnter command: ");

        scanf("%d", &command);

        switch (command)
        {
            case 1:
                equipment_init();
                break;

            case 2:
                equipment_start();
                break;

            case 3:
                equipment_stop();
                break;

            case 4:
                printf("Exiting Equipment Controller.\n");
                return 0;

            default:
                printf("Invalid command.\n");
                break;
        }
    }


    return 0;
}