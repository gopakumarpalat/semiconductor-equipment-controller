#include <stdio.h>
#include <string.h>
#include "equipment.h"

enum EquipmentState
{
    IDLE,
    READY,
    RUNNING
};

void equipment_started_message(Equipment *equipment)
{
    printf(">>> %s has started!\n", equipment->name);
}

void equipment_stop_message(Equipment *equipment)
{
    printf(">>> %s has stopped!\n", equipment->name);
}

void equipment_alarm_message( Equipment *equipment, const char *message)
{
    printf( ">>> %s ALARM: %s!\n", equipment->name, message);
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
    
    //Register the alarm callback for ETCH01.
    equipment_set_alarm_callback( &equipment_list[0], equipment_alarm_message);

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

    equipment_raise_alarm( &equipment_list[0], "Temperature too high");

    printf("\nFinal states:\n");

    for (int i = 0; i < 4; i++)
    {
        printf("%s: ", equipment_list[i].name);
        equipment_print_state(&equipment_list[i]);
    }

    return 0;
}