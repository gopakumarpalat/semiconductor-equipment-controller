#include <stdio.h>
#include <string.h>
#include "equipment.h"

enum EquipmentState
{
    IDLE,
    READY,
    RUNNING
};

int main()
{
    Equipment equipment_list[3];

    equipment_create(&equipment_list[0], 101, "ETCH01");
    equipment_create(&equipment_list[1], 102, "CVD01");
    equipment_create(&equipment_list[2], 103, "CMP01");

    printf("=================================\n");
    printf(" Semiconductor Equipment Control\n");
    printf("=================================\n\n");

   

    for(int i = 0; i < 3; i++)
    {
        printf("Equipment %d:\n", i + 1);
        printf("ID   : %d\n", equipment_list[i].id);
        printf("Name : %s\n", equipment_list[i].name);
        equipment_print_state(&equipment_list[i]);
        printf("\n");
    }

    printf("\nStarting ETCH01...\n");

    equipment_init(&equipment_list[0]);
    equipment_start(&equipment_list[0]);

    printf("\nFinal states:\n");

    for (int i = 0; i < 3; i++)
    {
        printf("%s: ", equipment_list[i].name);
        equipment_print_state(&equipment_list[i]);
    }

    return 0;
}