#include<stdio.h>
#include "equipment.h"

static EquipmentState current_state = EQUIPMENT_IDLE;

void equipment_init()
{
    current_state = EQUIPMENT_READY;

    printf("Equipment initialized.\n");
}

void equipment_start()
{
    if(current_state == EQUIPMENT_READY )
    {
        current_state = EQUIPMENT_RUNNING;
        printf("Equipment started.\n");

    }
    else
    {
        printf("Cannot start equipment.\n");
    }

}

void equipment_stop()
{
    if (current_state == EQUIPMENT_RUNNING)
    {
        current_state = EQUIPMENT_READY;

        printf("Equipment stopped.\n");
    }
    else
    {
        printf("Cannot stop equipment.\n");
    }
}

void equipment_print_state()
{
    switch (current_state)
    {
        case EQUIPMENT_IDLE:
            printf("State: IDLE\n");
            break;
        
        case EQUIPMENT_READY:
            printf("State: READY\n");
            break;

        case EQUIPMENT_RUNNING:
            printf("State: RUNNING\n");
            break;

        default:
            break;
    }

}