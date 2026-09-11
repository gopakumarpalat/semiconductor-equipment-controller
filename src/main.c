#include <stdio.h>
#include <string.h>
#include "equipment.h"

enum EquipmentState
{
    IDLE,
    READY,
    RUNNING
};

void equipment_started_message(void)
{
    printf(">>> Equipment has started!\n");
}

void equipment_stop_message(void)
{
    printf(">>> Equipment has stopped!\n");
}

void equipment_alarm_message(void)
{
    printf(">>> Equipment alarm started!\n");
}

int main()
{
    Equipment equipment_list[4];

    equipment_create(&equipment_list[0], 101, "ETCH01");
    equipment_create(&equipment_list[1], 102, "CVD01");
    equipment_create(&equipment_list[2], 103, "CMP01");
    equipment_create(&equipment_list[3], 104, "PVD01");

    printf("=================================\n");
    printf(" Semiconductor Equipment Control\n");
    printf("=================================\n\n");


    // Set equipment_started_message as a callback function of equipment
    equipment_set_start_callback( &equipment_list[0], equipment_started_message );
    equipment_set_start_callback( &equipment_list[3], equipment_started_message );

    //Register the stop callback for ETCH01.
    equipment_set_stop_callback( &equipment_list[0], equipment_stop_message );
   

    for(int i = 0; i < 4; i++)
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

    printf("\nStoping ETCH01...\n");
    equipment_stop(&equipment_list[0]);

    printf("\nStarting PVD01...\n");
    equipment_init(&equipment_list[3]);
    equipment_start(&equipment_list[3]);

    printf("\nFinal states:\n");

    for (int i = 0; i < 4; i++)
    {
        printf("%s: ", equipment_list[i].name);
        equipment_print_state(&equipment_list[i]);
    }

    return 0;
}